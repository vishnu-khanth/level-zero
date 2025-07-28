<%!
import re
from templates import helper as th

def get_enum_function_name(enum_type):
    """Convert enum type name to pre-generated assignment function name"""
    if not enum_type or not isinstance(enum_type, str):
        return None
    
    # Keep the full enum name including _t suffix for consistency
    return f"assign_{enum_type}"

def is_output_param(param):
    """Check if parameter is an output parameter"""
    desc = param.get('desc', '').lower()
    return '[out]' in desc or '[in,out]' in desc

def is_input_param(param):
    """Check if parameter is an input parameter"""
    desc = param.get('desc', '').lower()
    return '[in]' in desc or '[in,out]' in desc

def is_pointer_param(param_type):
    """Check if parameter is a pointer"""
    return '*' in param_type

def get_base_type(param_type):
    """Extract base type without pointer/const decorations"""
    return param_type.replace('*', '').replace('const ', '').strip()

# Global lookup tables - built once per template generation
_enum_lookup_table = None
_struct_lookup_table = None

def build_enum_lookup_table(meta):
    """Build a fast lookup table of all enum types from input.json"""
    enum_types = set()
    
    if 'enum' not in meta:
        return enum_types
    
    # Add all enum names from the specification
    for enum_name in meta['enum'].keys():
        # Convert template names to actual API names using th.subt()
        for namespace_key, prefix in [('ze', '$x_'), ('zes', '$s_'), ('zet', '$t_')]:
            if enum_name.startswith(prefix):
                # Use th.subt() for proper template substitution
                tags_map = {'$x': 'ze', '$s': 'zes', '$t': 'zet'}
                actual_name = th.subt(namespace_key, tags_map, enum_name)
                enum_types.add(actual_name)
    
    return enum_types

def build_struct_lookup_table(meta):
    """Build a fast lookup table of all structure types from input.json"""
    if 'struct' not in meta:
        return {}
    return meta['struct']

def is_enum_type(type_name, meta=None):
    """Check if type is an enum using spec-based lookup table"""
    global _enum_lookup_table
    
    if not type_name or not isinstance(type_name, str):
        return False
    
    # Build lookup table once
    if _enum_lookup_table is None and meta is not None:
        _enum_lookup_table = build_enum_lookup_table(meta)
    
    # Use lookup table for 100% accurate enum detection
    if _enum_lookup_table is not None:
        return type_name in _enum_lookup_table
    
    # Should never reach here since meta is always provided
    return False

def is_struct_type(type_name, meta=None):
    """Check if type is a structure using spec-based lookup table"""
    global _struct_lookup_table
    
    if not type_name or not isinstance(type_name, str):
        return False
    
    # Build lookup table once
    if _struct_lookup_table is None and meta is not None:
        _struct_lookup_table = build_struct_lookup_table(meta)
    
    # Convert to template name for lookup
    template_name = convert_to_template_name(type_name)
    return template_name in _struct_lookup_table

def convert_to_template_name(type_name):
    """Convert API type name to template name (ze_* → $x_*)"""
    if not type_name or not isinstance(type_name, str):
        return type_name
    if type_name.startswith('$'):
        return type_name  # Already a template name
    elif type_name.startswith('zes_'):
        return '$s_' + type_name[4:]  # System Management API
    elif type_name.startswith('zet_'):
        return '$t_' + type_name[4:]  # Tools API
    elif type_name.startswith('ze_'):
        return '$x_' + type_name[3:]  # Core API
    else:
        return type_name

def get_struct_members(struct_name, meta, n, tags):
    """Get members of a structure from meta data"""
    global _struct_lookup_table
    
    if not struct_name or not meta:
        return []
    
    # Build lookup table if needed
    if _struct_lookup_table is None:
        _struct_lookup_table = build_struct_lookup_table(meta)
    
    # Convert to template name for lookup
    template_name = convert_to_template_name(struct_name)
    
    if template_name not in _struct_lookup_table:
        return []
    
    struct_info = _struct_lookup_table[template_name]
    members = []
    
    for member in struct_info.get('members', []):
        if member and member.get('name') and member.get('type'):
            member_name = th.subt(n, tags, member.get('name', ''))
            member_type = th.subt(n, tags, member.get('type', ''))
            if member_name and member_type:
                members.append({
                    'name': member_name,
                    'type': member_type,
                    'base_type': get_base_type(member_type),
                    'is_pointer': is_pointer_param(member_type)
                })
    
    return members

def generate_struct_enum_assignments(param_name, struct_name, meta, n, tags, processed_structs=None, is_param_pointer=True, is_param_const=False):
    """Generate enum assignments for all enum members in a structure (recursive)"""
    if processed_structs is None:
        processed_structs = set()
    
    # Prevent infinite recursion for self-referencing structures
    if struct_name in processed_structs:
        return []
    
    processed_structs.add(struct_name)
    assignments = []
    
    try:
        members = get_struct_members(struct_name, meta, n, tags)
        
        for member in members:
            member_name = member['name']
            member_type = member['type']
            base_type = member['base_type']
            is_pointer = member['is_pointer']
            
            # Determine correct access operator
            access_op = "->" if is_param_pointer else "."
            
            if is_enum_type(base_type, meta) and not is_param_const:
                # Direct enum assignment (skip if parameter is const)
                enum_func = get_enum_function_name(base_type)
                if enum_func:
                    assignments.append(f"{enum_func}(&{param_name}{access_op}{member_name});")
            
            elif is_struct_type(base_type, meta) and not is_pointer:
                # Nested structure - recurse into its members  
                # The nested struct member is not a pointer, so next level uses direct access
                nested_assignments = generate_struct_enum_assignments(
                    f"{param_name}{access_op}{member_name}", base_type, meta, n, tags, processed_structs.copy(), False, is_param_const
                )
                assignments.extend(nested_assignments)
            
            elif is_pointer and 'pNext' in member_name and not is_param_const:
                # Extension chain pointer - set to nullptr (skip if const)
                assignments.append(f"{param_name}{access_op}{member_name} = nullptr;")
            
            # Note: Other types (basic types, void*, handle arrays) are handled by memset
    
    except Exception:
        # If anything goes wrong, return empty list to avoid breaking generation
        pass
    
    processed_structs.discard(struct_name)
    return assignments

%><%
    n=namespace
    N=n.upper()

    x=tags['$x']
    X=x.upper()
%>/*
 *
 * Copyright (C) 2019-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 * @file ${name}.cpp
 *
 */
#include "${x}_null.h"
#include "${n}_enum_assignments.h"
#include <cstring>

namespace driver
{
    %for obj in th.extract_objs(specs, r"function"):
    ///////////////////////////////////////////////////////////////////////////////
    <%
        fname = th.make_func_name(n, tags, obj)
    %>/// @brief Intercept function for ${fname}
    %if 'condition' in obj:
    #if ${th.subt(n, tags, obj['condition'])}
    %endif
    __${x}dlllocal ${x}_result_t ${X}_APICALL
    ${fname}(
        %for line in th.make_param_lines(n, tags, obj):
        ${line}
        %endfor
        )
    {
        ${x}_result_t result = ${X}_RESULT_SUCCESS;

        // if the driver has created a custom function, then call it instead of using the generic path
        auto ${th.make_pfn_name(n, tags, obj)} = context.${n}DdiTable.${th.get_table_name(n, tags, obj)}.${th.make_pfn_name(n, tags, obj)};
        if( nullptr != ${th.make_pfn_name(n, tags, obj)} )
        {
            result = ${th.make_pfn_name(n, tags, obj)}( ${", ".join(th.make_param_lines(n, tags, obj, format=["name"]))} );
        }
        else
        {
            // generic implementation
            %if re.match("Init", obj['name']):
            %if re.match("InitDrivers", obj['name']):
            auto driver_type = getenv_string( "ZEL_TEST_NULL_DRIVER_TYPE" );
            if (std::strcmp(driver_type.c_str(), "GPU") == 0) {
                if (!(desc->flags & ZE_INIT_DRIVER_TYPE_FLAG_GPU)) {
                    return ${X}_RESULT_ERROR_UNINITIALIZED;
                }
            }
            if (std::strcmp(driver_type.c_str(), "NPU") == 0) {
                if (!(desc->flags & ZE_INIT_DRIVER_TYPE_FLAG_NPU)) {
                    return ${X}_RESULT_ERROR_UNINITIALIZED;
                }
            }

            if (phDrivers == nullptr) {
                *pCount = 1;
            }
            %else:
            auto driver_type = getenv_string( "ZEL_TEST_NULL_DRIVER_TYPE" );
            if (std::strcmp(driver_type.c_str(), "GPU") == 0) {
                if (!(flags & ZE_INIT_FLAG_GPU_ONLY)) {
                    return ${X}_RESULT_ERROR_UNINITIALIZED;
                }
            }
            if (std::strcmp(driver_type.c_str(), "NPU") == 0) {
                if (!(flags & ZE_INIT_FLAG_VPU_ONLY)) {
                    return ${X}_RESULT_ERROR_UNINITIALIZED;
                }
            }
            %endif
            %endif
            %for item in th.get_loader_epilogue(n, tags, obj, meta):
            %if 'range' in item:
            for( size_t i = ${item['range'][0]}; ( nullptr != ${item['name']} ) && ( i < ${item['range'][1]} ); ++i )
                ${item['name']}[ i ] = reinterpret_cast<${item['type']}>( context.get() );
            %elif not item['release']:
            %if item['optional']:
            if( nullptr != ${item['name']} ) *${item['name']} = reinterpret_cast<${item['type']}>( context.get() );
            %else:
            *${item['name']} = reinterpret_cast<${item['type']}>( context.get() );
            %endif
            %endif

            %endfor
            
            // SYSTEMATIC ENUM ASSIGNMENT LOGIC
            // Process each parameter according to direction and type
            %for param in obj['params']:
<%
    param_name = th.subt(n, tags, param.get('name', ''))
    param_type = th.subt(n, tags, param.get('type', ''))
    
    if not param_name or not param_type:
        continue
    
    is_input = is_input_param(param)
    is_output = is_output_param(param)
    is_pointer = is_pointer_param(param_type)
    is_const = 'const' in param_type
    base_type = get_base_type(param_type)
    is_enum = is_enum_type(base_type, meta)
    is_struct = is_struct_type(base_type, meta)
    
    # Generate structure member assignments if it's an output structure (skip if const)
    struct_assignments = []
    if is_output and is_pointer and is_struct and not is_const:
        struct_assignments = generate_struct_enum_assignments(param_name, base_type, meta, n, tags, None, True, is_const)
%>
            %if is_input and is_pointer:
            // INPUT: Dummy read operation (pointer)
            if( nullptr != ${param_name} )
            {
                %if 'void*' in param_type or 'const void*' in param_type:
                // Void pointer - cannot dereference
                volatile auto temp_voidptr = ${param_name}; (void)temp_voidptr;
                %else:
                volatile auto temp_read = *${param_name}; (void)temp_read;
                %endif
            }
            %endif
            %if is_input and not is_pointer:
            // INPUT: Dummy read operation (value)
            volatile auto temp_read_${param_name} = ${param_name}; (void)temp_read_${param_name};
            %endif
            %if is_output and is_pointer:
            // OUTPUT: Process based on parameter type
            if( nullptr != ${param_name} )
            {
                %if is_enum and not is_const:
                // Direct enum assignment (skip if const)
                ${get_enum_function_name(base_type)}(${param_name});
                %elif is_struct and struct_assignments:
                // Structure with enum members - iterate and assign
                %for assignment in struct_assignments:
                ${assignment}
                %endfor
                %elif 'pCount' in param_name or 'count' in param_name.lower():
                // Count parameter - skip, handled by loader epilogue
                %elif 'void*' in param_type:
                // Void pointer - cannot dereference
                volatile auto temp_voidptr = ${param_name}; (void)temp_voidptr;
                %elif not is_const:
                // Basic type or structure without enums - memset handles it (skip if const)
                memset(${param_name}, 0, sizeof(*${param_name}));
                %else:
                // Const parameter - can only do dummy read
                volatile auto temp_read_const = *${param_name}; (void)temp_read_const;
                %endif
            }
            %endif
            %endfor
        }

        return result;
    }
    %if 'condition' in obj:
    #endif // ${th.subt(n, tags, obj['condition'])}
    %endif

    %endfor
} // namespace driver

#if defined(__cplusplus)
extern "C" {
#endif

%for tbl in th.get_pfntables(specs, meta, n, tags):
///////////////////////////////////////////////////////////////////////////////
/// @brief Exported function for filling application's ${tbl['name']} table
///        with current process' addresses
///
/// @returns
///     - ::${X}_RESULT_SUCCESS
///     - ::${X}_RESULT_ERROR_INVALID_NULL_POINTER
///     - ::${X}_RESULT_ERROR_UNSUPPORTED_VERSION
${X}_DLLEXPORT ${x}_result_t ${X}_APICALL
${tbl['export']['name']}(
    %for line in th.make_param_lines(n, tags, tbl['export']):
    ${line}
    %endfor
    )
{
    if( nullptr == pDdiTable )
        return ${X}_RESULT_ERROR_INVALID_NULL_POINTER;

    if( driver::context.version < version )
        return ${X}_RESULT_ERROR_UNSUPPORTED_VERSION;

    ${x}_result_t result = ${X}_RESULT_SUCCESS;

% if tbl['name'] == 'Global' and n == 'ze':
    pDdiTable->pfnInit                                   = driver::zeInit;

    auto missing_api = getenv_string( "ZEL_TEST_MISSING_API" );
    auto missing_api_driver_id = getenv_string( "ZEL_TEST_MISSING_API_DRIVER_ID" );
    std::string zeInitDriversWithNullDriverId = "zeInitDrivers:" + std::to_string(ZEL_NULL_DRIVER_ID);
    if (std::strcmp(missing_api_driver_id.c_str(), zeInitDriversWithNullDriverId.c_str()) == 0) {
        pDdiTable->pfnInitDrivers                            = nullptr;
    } else if (std::strcmp(missing_api.c_str(), "zeInitDrivers") == 0) {
        pDdiTable->pfnInitDrivers                            = nullptr;
    } else {
        pDdiTable->pfnInitDrivers                            = driver::zeInitDrivers;
    }
%else:
    %for obj in tbl['functions']:
    %if 'condition' in obj:
#if ${th.subt(n, tags, obj['condition'])}
    %endif
    pDdiTable->${th.append_ws(th.make_pfn_name(n, tags, obj), 41)} = driver::${th.make_func_name(n, tags, obj)};
    %if 'condition' in obj:
#else
    pDdiTable->${th.append_ws(th.make_pfn_name(n, tags, obj), 41)} = nullptr;
#endif
    %endif

    %endfor
%endif
    return result;
}

%endfor
#if defined(__cplusplus)
};
#endif
