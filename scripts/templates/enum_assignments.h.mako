<%!
import re
from templates import helper as th

# Extract all enums for a specific namespace from meta.enum
def extract_namespace_enums(meta, namespace, tags):
    namespace_enums = []
    
    # Debug: print what we have
    print(f"DEBUG: Looking for namespace '{namespace}'")
    
    if 'enum' not in meta:
        print("DEBUG: No 'enum' section in meta")
        return namespace_enums
    
    enums = meta['enum']
    print(f"DEBUG: Total enums in meta: {len(enums)}")
    
    # Show first 10 enum names to understand pattern
    enum_names = list(enums.keys())[:10]
    print(f"DEBUG: Sample enum names: {enum_names}")
    
    # Known problematic enum types that don't exist in headers
    excluded_enums = {
        'ze_calculate_multiple_metrics_exp_version_t',
        'ze_metric_global_timestamps_exp_version_t'
    }
    
    for enum_name, enum_data in enums.items():
        # Convert template enum name to actual enum name using Level Zero's template substitution
        actual_enum_name = th.subt(namespace, tags, enum_name)
        
        # Skip excluded enum types that cause compilation errors
        if actual_enum_name in excluded_enums:
            print(f"DEBUG: Skipping excluded enum '{actual_enum_name}' (not in API headers)")
            continue
        
        # Check if this enum belongs to the requested namespace
        # After template substitution, enum names should start with namespace prefix
        if actual_enum_name.startswith(f'{namespace}_'):
            print(f"DEBUG: Found enum '{enum_name}' -> '{actual_enum_name}' for namespace '{namespace}'")
            if 'etors' in enum_data and enum_data['etors']:
                # Create a compatible enum object structure with the actual enum name
                enum_obj = {
                    'name': actual_enum_name,
                    'etors': enum_data['etors'],
                    'desc': enum_data.get('desc', ''),
                    'type': 'enum'
                }
                namespace_enums.append(enum_obj)
    
    print(f"DEBUG: Total enums found for '{namespace}': {len(namespace_enums)}")
    return namespace_enums

# Generate function name for enum assignment
def get_enum_function_name(enum_name, namespace):
    # Keep the full enum name including _t suffix for consistency
    return "assign_" + enum_name

%>
/*
 * Copyright (C) 2019-2025 Intel Corporation
 * 
 * SPDX-License-Identifier: MIT
 * 
 * Auto-generated enum assignment functions for Level Zero null driver testing.
 * Generated from Level Zero API specification.
 */

#ifndef ${namespace.upper()}_ENUM_ASSIGNMENTS_H
#define ${namespace.upper()}_ENUM_ASSIGNMENTS_H

#include "${namespace}_api.h"
%if namespace in ['zes', 'zet']:
#include "ze_enum_assignments.h"
%endif

<%
# Get enums for this namespace
enums = extract_namespace_enums(meta, namespace, tags)
%>

// Enum assignment functions for ${namespace} namespace
% for enum in enums:
<%
    func_name = get_enum_function_name(enum['name'], namespace)
%>
% if func_name:
void ${func_name}(${enum['name']}* pEnum);
% endif
% endfor

#endif // ${namespace.upper()}_ENUM_ASSIGNMENTS_H
