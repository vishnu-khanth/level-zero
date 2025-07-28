<%!
import re
from templates import helper as th

# Extract all enums for a specific namespace from meta.enum
def extract_namespace_enums(meta, namespace, tags):
    namespace_enums = []
    
    if 'enum' not in meta:
        return namespace_enums
    
    enums = meta['enum']
    
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
            continue
        
        # Check if this enum belongs to the requested namespace
        # After template substitution, enum names should start with namespace prefix
        if actual_enum_name.startswith(f'{namespace}_'):
            if 'etors' in enum_data and enum_data['etors']:
                # Create a compatible enum object structure with the actual enum name
                enum_obj = {
                    'name': actual_enum_name,
                    'etors': enum_data['etors'],
                    'desc': enum_data.get('desc', ''),
                    'type': 'enum'
                }
                namespace_enums.append(enum_obj)
    
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

#include "${namespace}_enum_assignments.h"

<%
# Get enums for this namespace
enums = extract_namespace_enums(meta, namespace, tags)
%>

// Enum assignment function implementations for ${namespace} namespace
% for enum in enums:
<%
    func_name = get_enum_function_name(enum['name'], namespace)
%>
% if func_name:
void ${func_name}(${enum['name']}* pEnum) {
    if (pEnum == nullptr) return;
    
    // Assign all enum values for comprehensive testing
% for etor in enum.get('etors', []):
<%
    # Handle both string and dict formats for etors
    if isinstance(etor, dict):
        etor_name = etor.get('name', '')
    else:
        etor_name = str(etor)
    
    # Convert template enum value name to actual enum value name
    actual_etor_name = th.subt(namespace, tags, etor_name)
%>
    *pEnum = ${actual_etor_name};
% endfor
}

% endif
% endfor
