import datetime
import sys
import os
import time
import re
from genericpath import isfile
from dataclasses import dataclass
from enum import Enum

os.chdir(os.path.dirname(os.path.abspath(__file__)))
global srcdir; srcdir = "..\\" + sys.argv[1]
global gendir; gendir = "..\\" + sys.argv[2]
global userlib_path; userlib_path = gendir + "UserLib.cpp"

global srcmap; srcmap = dict()
global found_files; found_files = list()

class VariableType(Enum):
    VAR_TYPE_NULL     = -1
    VAR_TYPE_INT      = 1
    VAR_TYPE_UINT32_T = 2
    VAR_TYPE_UINT64_T = 3
    VAR_TYPE_FLOAT    = 4
    VAR_TYPE_DOUBLE   = 5
    VAR_TYPE_BOOL     = 6
    VAR_TYPE_STRING   = 7

def var_type_to_str(var_type: VariableType):
    if var_type.value == 1:
        return "TYPE_INT"
    elif var_type.value == 2:
        return "TYPE_UINT32_T"
    elif var_type.value == 3:
        return "TYPE_UINT64_T"
    elif var_type.value == 4:
        return "TYPE_FLOAT"
    elif var_type.value == 5:
        return "TYPE_DOUBLE"
    elif var_type.value == 6:
        return "TYPE_BOOL"
    elif var_type.value == 7:
        return "TYPE_STRING"
    elif var_type.value == -1:
        return "TYPE_NULL"

def var_type_to_cpp(var_type: VariableType):
    if var_type.value == 1:
        return "int"
    elif var_type.value == 2:
        return "uint32_t"
    elif var_type.value == 3:
        return "uint64_t"
    elif var_type.value == 4:
        return "float"
    elif var_type.value == 5:
        return "double"
    elif var_type.value == 6:
        return "bool"
    elif var_type.value == 7:
        return "std::string"

@dataclass
class VariableSpecification:
    name: str
    var_type: VariableType

    def __init__(self, name: str, var_type: VariableType):
        self.name = name
        self.var_type = var_type

@dataclass
class ScriptSpecification:
    variables: list
    functions: list

    def __init__(self, variables: list, functions: list):
        self.variables = variables
        self.functions = functions

@dataclass
class ComponentSpecification:
    variables: list

    def __init__(self, variables: list):
        self.variables = variables

global domains; domains = list()
global renderers; renderers = list()
global scripts; scripts = dict()
global components; components = dict()
global includes; includes = set()

def save_src_map():
    f = open("srcmap", "w")
    
    content = str()
    srcmaplen = len(srcmap)-1
    for entry in srcmap:
        content += str(entry) + "|" + str(srcmap[entry]) + "\n"
    content = content[:len(content)-1]
    
    f.write(content)
    f.close()
    return

def read_src_map():
    if not os.path.isfile("srcmap"):
        return

    f = open("srcmap", "r")
    lines = f.readlines()

    for line in lines:
        line = line.strip("\n")
        values = line.split('|')
        srcmap[values[0]] = float(values[1])
    
    f.close()
    return

def remove_raw_strings(file_content):
    rstr_found = False
    match = re.search("R\"\((?s:.*?)\)\"", file_content, re.MULTILINE)
    new_content = str()
    if match is not None:
        rstr_found = True
        new_content = file_content[0:match.start()] + " " + file_content[match.end():len(file_content)]
        return remove_raw_strings(new_content)

    return file_content

def remove_strings(file_content):
    str_found = False
    match = re.search("\".*\"", file_content)
    new_content = str()
    if match is not None:
        str_found = True
        new_content = file_content[0:match.start()] + " " + file_content[match.end():len(file_content)]
        return remove_strings(new_content)
    
    return file_content

def remove_comments(file_content):
    comment_found = False
    match = re.search("//.*", file_content)
    
    new_content = str()
    if match is not None:
        comment_found = True
        new_content = file_content[0:match.start()] + " " + file_content[match.end():len(file_content)]
        return remove_comments(new_content)
    
    return file_content

def remove_multi_comments(file_content):
    comment_found = False
    match = re.search("/[*](?s:.*?)[*]/", file_content, re.MULTILINE)
    
    new_content = str()
    if match is not None:
        comment_found = True
        new_content = file_content[0:match.start()] + " " + file_content[match.end():len(file_content)]
        return remove_multi_comments(new_content)
    
    return file_content

def scan_variable(variable_str):
    spec = VariableSpecification("", VariableType.VAR_TYPE_NULL)

    int_match = re.search(r"^int(\s+).+(\s+)*", variable_str)
    if int_match:
        spec.var_type = VariableType.VAR_TYPE_INT
        spec.name = int_match.group(0).split()[1]
        return spec
    
    uint32_t_match = re.search(r"^uint32_t(\s+).+(\s+)*", variable_str)
    if uint32_t_match:
        spec.var_type = VariableType.VAR_TYPE_UINT32_T
        spec.name = uint32_t_match.group(0).split()[1]
        return spec
    
    uint64_t_match = re.search(r"^uint64_t(\s+).+(\s+)*", variable_str)
    if uint64_t_match:
        spec.var_type = VariableType.VAR_TYPE_UINT64_T
        spec.name = uint64_t_match.group(0).split()[1]
        return spec
    
    float_match = re.search(r"^float(\s+).+(\s+)*", variable_str)
    if float_match:
        spec.var_type = VariableType.VAR_TYPE_FLOAT
        spec.name = float_match.group(0).split()[1]
        return spec
    
    double_match = re.search(r"^double(\s+).+(\s+)*", variable_str)
    if double_match:
        spec.var_type = VariableType.VAR_TYPE_DOUBLE
        spec.name = double_match.group(0).split()[1]
        return spec
    
    bool_match = re.search(r"^bool(\s+).+(\s+)*", variable_str)
    if bool_match:
        spec.var_type = VariableType.VAR_TYPE_BOOL
        spec.name = bool_match.group(0).split()[1]
        return spec
    
    std_string_match = re.search(r"^std::string(\s+).+(\s+)*", variable_str)
    string_match = re.search(r"^string(\s+).+(\s+)*", variable_str)
    if std_string_match or string_match:
        spec.var_type = VariableType.VAR_TYPE_STRING
        if std_string_match:
            spec.name = std_string_match.group(0).split()[1]
        else:
            spec.name = string_match.group(0).split()[1]
        return spec

    return spec

def scan_class(class_name, class_src, is_script):
    open_brace_count = 0
    closed_brace_count = 0
    class_src_end = 0
    
    for i, char in enumerate(class_src):
        if char == '{':
            open_brace_count += 1
        elif char == '}':
            closed_brace_count += 1
        
        if open_brace_count >= 1 and open_brace_count == closed_brace_count:
            class_src_end = i
            break
        elif closed_brace_count > open_brace_count:
            print("Bad class source code!")
            return

    processed_class_src = class_src[:class_src_end]

    om_param_matches = re.finditer(r"OM_PARAM;*", processed_class_src, re.MULTILINE)
    om_begin_param_group_matches = re.finditer(r"OM_BEGIN_PARAM_GROUP;*", processed_class_src, re.MULTILINE)
    om_end_param_group_matches = re.finditer(r"OM_END_PARAM_GROUP;*", processed_class_src, re.MULTILINE)
    
    var_specs = list()

    for m in om_param_matches:
        var_str = processed_class_src[m.end():processed_class_src.find(';', m.end())+1]
        var_str = var_str.replace("\n", " ")
        var_str = var_str.strip()
        var_str = var_str[:var_str.find('=')]
        var_specs.append(scan_variable(var_str))

    begin_param_group_list = list()
    for m in om_begin_param_group_matches:
        begin_param_group_list.append(m)

    end_param_group_list = list()
    for m in om_end_param_group_matches:
        end_param_group_list.append(m)

    if len(begin_param_group_list) != len(end_param_group_list):
        print("OM_BEGIN_PARAM_GROUP / OM_END_PARAM_GROUP mismatch in script \"" + class_name + "\"!")
        return

    for i in range(len(begin_param_group_list)):
        group_str = processed_class_src[begin_param_group_list[i].end():end_param_group_list[i].start()]
        variables = group_str.split(";")
        # Delete empty string that comes after last ';'
        variables.pop()
        for variable in variables:
            variable = variable.replace("\n", " ")
            variable = variable.strip()
            var_specs.append(scan_variable(variable))
    
    # Script
    if is_script:
        functions = list()
        script_spec = ScriptSpecification(var_specs, functions)
        scripts[class_name] = script_spec
    # Component
    else:
        component_spec = ComponentSpecification(var_specs)
        components[class_name] = component_spec

def scan_component(component_src):
    open_brace_match = re.search("{", component_src, re.MULTILINE)
    if open_brace_match.group(0) == "":
        return
    component_name_str = component_src[:open_brace_match.start()]
    component_name_str = component_name_str.strip()

    component_struct_match = re.search(r"^struct(\s+).+(\s+)*", component_name_str, re.MULTILINE)
    component_class_match = re.search(r"^class(\s+).+(\s+)*", component_name_str, re.MULTILINE)
    if component_struct_match.group(0) == "" and component_struct_match.group(0) == "":
        return
    
    component_name = str()
    if component_struct_match.group(0) != "":
        component_name = component_struct_match.group(0).split()[1]
    elif component_class_match.group(0) != "":
        component_name = component_class_match.group(0).split()[1]
    component_name = component_name.replace("\n", "")
    component_name = component_name.replace(":", "")
    component_name = component_name.replace(";", "")

    scan_class(component_name, component_src, False)

def scan_file(file_path):
    f = open(file_path, "r")
    file_content = f.read()
    
    no_domains = False
    no_renderers = False
    no_scripts = False
    no_components = False

    if "Domain" not in file_content:
        no_domains = True
    if "RendererDomain" not in file_content:
        no_renderers = True
    if "ScriptableEntity" not in file_content:
        no_scripts = True
    if "OM_COMPONENT" not in file_content:
        no_components = True
    
    if no_domains and no_renderers and no_scripts and no_components:
        return
    
    processed_content = remove_raw_strings(file_content)
    processed_content = remove_strings(processed_content)
    processed_content = remove_comments(processed_content)
    processed_content = remove_multi_comments(processed_content)
    
    domain_matches = re.finditer(r"class(\s+).+(\s+)*:(\s+)*Domain", processed_content, re.MULTILINE)
    domain_namespace_matches = re.finditer(r"class(\s+).+(\s+)*:(\s+)*Omni(\s+)*::(\s+)*Domain", processed_content, re.MULTILINE)

    renderer_matches = re.finditer(r"class(\s+).+(\s+)*:(\s+)*RendererDomain", processed_content, re.MULTILINE)
    renderer_namespace_matches = re.finditer(r"class(\s+).+(\s+)*:(\s+)*Omni(\s+)*::(\s+)*RendererDomain", processed_content, re.MULTILINE)
    
    script_matches = re.finditer(r"class(\s+).+(\s+)*:(\s+)*ScriptableEntity", processed_content, re.MULTILINE)
    script_namespace_matches = re.finditer(r"class(\s+).+(\s+)*:(\s+)*Omni(\s+)*::(\s+)*ScriptableEntity", processed_content, re.MULTILINE)

    component_matches = re.finditer(r"OM_COMPONENT;*", processed_content, re.MULTILINE)

    domain_match_list = list()
    renderer_match_list = list()
    script_match_list = list()
    component_match_list = list()
    
    for m in domain_matches:
        domain_match_list.append(m)
    
    for m in renderer_matches:
        renderer_match_list.append(m)
    
    for m in script_matches:
        script_match_list.append(m)
    
    for m in component_matches:
        component_match_list.append(m)
        
    domain_match_list_starts = list()
    renderer_match_list_starts = list()
    script_match_list_starts = list()
    
    for m in domain_match_list:
        domain_match_list_starts.append(m.start())

    for m in domain_namespace_matches:
        if m.start() not in domain_match_list_starts:
            domain_match_list.append(m)
    
    for m in renderer_match_list:
        renderer_match_list_starts.append(m.start())
    
    for m in renderer_namespace_matches:
        if m.start() not in renderer_match_list_starts:
            renderer_match_list.append(m)

    for m in script_match_list:
        script_match_list_starts.append(m.start())

    for m in script_namespace_matches:
        if m.start() not in script_match_list_starts:
            script_match_list.append(m)

    for m in domain_match_list:
        processed_str = m.group(0).replace("\n", " ")
        processed_str = processed_str.replace(":", " ")
        class_name = processed_str.split()[1]
        domains.append(class_name)
    
    for m in renderer_match_list:
        processed_str = m.group(0).replace("\n", " ")
        processed_str = processed_str.replace(":", " ")
        class_name = processed_str.split()[1]
        renderers.append(class_name)
    
    for m in script_match_list:
        processed_str = m.group(0).replace("\n", " ")
        processed_str = processed_str.replace(":", " ")
        class_name = processed_str.split()[1]
        scan_class(class_name, processed_content[m.start():], True)
    
    for m in component_match_list:
        processed_str = m.group(0).replace("\n", " ")
        scan_component(processed_content[m.end():])

    if len(domain_match_list) != 0:
        includes.add(file_path[len(srcdir)-4:])
    
    if len(renderer_match_list) != 0:
        includes.add(file_path[len(srcdir)-4:])
    
    if len(script_match_list) != 0:
        includes.add(file_path[len(srcdir)-4:])
    
    if len(component_match_list) != 0:
        includes.add(file_path[len(srcdir)-4:])
    
    f.close()
    
    return

def update_srcmap(path):
    for filename in os.listdir(path):
        f = os.path.join(path, filename)
        if os.path.isfile(f):
            found_files.append(f)
            last_modified = os.path.getmtime(f)

            if f not in srcmap.keys():
                srcmap[f] = last_modified
            else:
                if srcmap[f] != last_modified:
                    srcmap[f] = last_modified
        # if directory
        else:
            update_srcmap(f)
    
    return

def scan_src():
    # Scan for modified files
    
    update_srcmap(srcdir)

    # Delete any srcmap entries that were not found
    
    entries_to_delete = list()
    for entry in srcmap:
        if entry not in found_files:
            entries_to_delete.append(entry)
    
    for entry in entries_to_delete:
        del srcmap[entry]

    # Scan inside files
    
    for entry in srcmap:
        scan_file(entry)
        
    processed_includes = set()
    for include in includes:
        processed_includes.add(include.replace('\\', '/'))
    
    includes.clear()

    for include in processed_includes:
        includes.add(include)

    save_src_map()
    
    return

def generate_code():
    # Includes
    
    includes_str = str()

    for include in includes:
        includes_str += "#include \"%s\"\n" % include
    includes_str = includes_str[0:len(includes_str)-1]

    # AllUserComponents definition

    all_user_components_str = str()

    for i, component_name in enumerate(components):
        if i == 0:
            all_user_components_str += ",\n"
            
        if i == len(components) - 1:
            all_user_components_str += "\t" + component_name
        else:
            all_user_components_str += "\t" + component_name + ",\n"

    # Hashes
    
    hashes_str = str()

    domain_hashes = list()
    for domain in domains:
        domain_hash = domain + "_Hash"
        hashes_str += "uint64_t " + domain_hash + ";\n\n"
        domain_hashes.append(domain_hash)

    renderer_hashes = list()
    for renderer in renderers:
        renderer_hash = renderer + "_Hash"
        hashes_str += "uint64_t " + renderer_hash + ";\n\n"
        renderer_hashes.append(renderer_hash)

    script_hashes = list()
    for script_name in scripts.keys():
        script = scripts[script_name]
        script_hash = script_name + "_Hash"
        hashes_str += "uint64_t " + script_hash + ";\n"
        variable_hashes = list()
        for variable in script.variables:
            variable_hash = script_name + "_" + variable.name + "_Hash"
            hashes_str += "uint64_t " + variable_hash + ";\n"
            variable_hashes.append(variable_hash)
        hashes_str += '\n'
        script_hashes.append((script_hash, variable_hashes))
    
    component_hashes = list()
    for component_name in components.keys():
        component = components[component_name]
        component_hash = component_name + "_Hash"
        hashes_str += "uint64_t " + component_hash + ";\n"
        variable_hashes = list()
        for variable in component.variables:
            variable_hash = component_name + "_" + variable.name + "_Hash"
            hashes_str += "uint64_t " + variable_hash + ";\n"
            variable_hashes.append(variable_hash)
        hashes_str += '\n'
        component_hashes.append((component_hash, variable_hashes))
    
    # Init function implementation

    init_str = str()

    init_str += "\t// Domains\n"
    init_str += "\tg_Domains = new std::unordered_map<uint64_t, std::string>();\n\n"

    for i, domain in enumerate(domains):
        init_str += "\t{\n"
        init_str += "\t\tstd::string name = \"" + domain + "\";\n"
        init_str += "\t\t" + domain_hashes[i] + " = MurmurHash64A(name.c_str(), name.length(), HASH_SEED);\n"
        init_str += "\t\t(*g_Domains)[" + domain_hashes[i] + "] = name;\n"
        init_str += "\t}\n\n"

    init_str += "\t// Domains\n\n"

    init_str += "\t// Renderers\n"
    init_str += "\tg_RendererDomains = new std::unordered_map<uint64_t, std::string>();\n\n"

    for i, renderer in enumerate(renderers):
        init_str += "\t{\n"
        init_str += "\t\tstd::string name = \"" + renderer + "\";\n"
        init_str += "\t\t" + renderer_hashes[i] + " = MurmurHash64A(name.c_str(), name.length(), HASH_SEED);\n"
        init_str += "\t\t(*g_RendererDomains)[" + renderer_hashes[i] + "] = name;\n"
        init_str += "\t}\n\n"

    init_str += "\t// Renderers\n\n"

    init_str += "\t// Scripts\n"
    init_str += "\tg_Scripts = new std::unordered_map<uint64_t, Omni::ScriptSpecification>();\n\n"

    for i, script_name in enumerate(scripts.keys()):
        init_str += "\t{\n"
        init_str += "\t\tstd::string name = \"" + script_name + "\";\n"
        init_str += "\t\tOmni::ScriptSpecification spec = Omni::ScriptSpecification(name);\n"

        script = scripts[script_name]

        if len(script.variables) > 0:
            init_str += '\n'

        for j, variable in enumerate(script.variables):
            init_str += "\t\t{\n"
            init_str += "\t\t\tstd::string varName = name + \"_" + variable.name + "\";\n"
            init_str += "\t\t\t" + script_hashes[i][1][j] + " = MurmurHash64A(varName.c_str(), varName.length(), HASH_SEED);\n"
            init_str += "\t\t\tspec.Variables[" + script_hashes[i][1][j] + "] = { \"" + script.variables[j].name + "\", Omni::ScriptVarType::" + var_type_to_str(script.variables[j].var_type) + " };\n"
            init_str += "\t\t}\n\n"
        
        init_str += "\t\t" + script_hashes[i][0] + " = MurmurHash64A(name.c_str(), name.length(), HASH_SEED);\n"
        init_str += "\t\t(*g_Scripts)[" + script_hashes[i][0] + "] = spec;\n"

        init_str += "\t}\n\n"
    
    init_str += "\t// Scripts\n\n"

    init_str += "\t// Components\n\n"
    init_str += "\tg_Components = new std::unordered_map<uint64_t, Omni::ComponentSpecification>();\n\n"

    for i, component_name in enumerate(components.keys()):
        init_str += "\t{\n"
        init_str += "\t\tstd::string name = \"" + component_name + "\";\n"
        init_str += "\t\tOmni::ComponentSpecification spec = Omni::ComponentSpecification(name);\n"

        component = components[component_name]

        if len(component.variables) > 0:
            init_str += '\n'
        
        for j, variable in enumerate(component.variables):
            init_str += "\t\t{\n"
            init_str += "\t\t\tstd::string varName = name + \"_" + variable.name + "\";\n"
            init_str += "\t\t\t" + component_hashes[i][1][j] + " = MurmurHash64A(varName.c_str(), varName.length(), HASH_SEED);\n"
            init_str += "\t\t\tspec.Variables[" + component_hashes[i][1][j] + "] = { \"" + component.variables[j].name + "\", Omni::ScriptVarType::" + var_type_to_str(component.variables[j].var_type) + " };\n"
            init_str += "\t\t}\n\n"
        
        init_str += "\t\t" + component_hashes[i][0] + " = MurmurHash64A(name.c_str(), name.length(), HASH_SEED);\n"
        init_str += "\t\t(*g_Components)[" + component_hashes[i][0] + "] = spec;\n"

        init_str += "\t}\n"
    
    init_str += "\t// Components\n"

    # GetDomain function implementation
    
    get_domain_str = str()
    for i, domain_name in enumerate(domains):
        if i == 0:
            get_domain_str += "\tif "
        else:
            get_domain_str += "\telse if "
        get_domain_str += "(domainID == " + domain_hashes[i] + ") { return static_cast<void*>(new " + domain_name + "); }\n"
    
    # GetRendererDomain function implementation
    
    get_renderer_str = str()
    for i, renderer_name in enumerate(renderers):
        if i == 0:
            get_renderer_str += "\tif "
        else:
            get_renderer_str += "\telse if "
        get_renderer_str += "(domainID == " + renderer_hashes[i] + ") { return static_cast<void*>(new " + renderer_name + "); }\n"

    # GetScript function implementation
    
    get_script_str = str()
    for i, script_name in enumerate(scripts.keys()):
        if i == 0:
            get_script_str += "\tif (scriptID == " + script_hashes[i][0] + ") { return static_cast<void*>(new " + script_name + "); }\n"
        else:
            get_script_str += "\telse if (scriptID == " + script_hashes[i][0] + ") { return static_cast<void*>(new " + script_name + "); }\n"

    # GetScriptVar function implementation
    
    get_script_var_str = str()
    for i, script_name in enumerate(scripts.keys()):
        if len(scripts[script_name].variables) <= 0:
            continue

        script = scripts[script_name]

        if get_script_var_str == "":
            get_script_var_str += "\tif "
        else:
            get_script_var_str += "\telse if "
        
        get_script_var_str += "(scriptID == " + script_hashes[i][0] + ")\n"
        get_script_var_str += "\t{\n"
        get_script_var_str += "\t\tauto scriptableEntity = static_cast<" + script_name + "*>(script);\n\n"

        for j, variable in enumerate(script.variables):
            if j == 0:
                get_script_var_str += "\t\tif "
            else:
                get_script_var_str += "\t\telse if "
            
            get_script_var_str += "(varID == " + script_hashes[i][1][j] + ") { return static_cast<void*>(&scriptableEntity->" + script.variables[j].name + "); }\n"

        get_script_var_str += "\t}\n"
    
    # GetScriptSpecs function implementation

    get_script_specs_str = str()
    for i, script_name in enumerate(scripts.keys()):
        script = scripts[script_name]

        get_script_specs_str += "\t{\n"
        if len(script.variables) > 0:
            get_script_specs_str += "\t\tstd::vector<Omni::VariableSpecification>* varSpecs = new std::vector<Omni::VariableSpecification>();\n"
        for j, variable in enumerate(script.variables):
            get_script_specs_str += "\t\tvarSpecs->emplace_back((*g_Scripts)[" + script_hashes[i][0] + "].Variables[" + script_hashes[i][1][j] + "]);\n"
        if len(script.variables) > 0:
            get_script_specs_str += "\t\tscriptSpecs->emplace_back(std::make_pair(" + script_hashes[i][0] + ", varSpecs));\n"
        else:
            get_script_specs_str += "\t\tscriptSpecs->emplace_back(std::make_pair(" + script_hashes[i][0] + ", nullptr));\n"
        get_script_specs_str += "\t}\n\n"

    # GetUserComponent function implementation

    get_user_component_str = str()
    for i, component_name in enumerate(components):
        if i == 0:
            get_user_component_str += "\tif (componentID == " + component_hashes[i][0] + ") { return static_cast<void*>(new " + component_name + "); }\n"
        else:
            get_user_component_str += "\telse if (componentID == " + component_hashes[i][0] + ") { return static_cast<void*>(new " + component_name + "); }\n"

    # HasUserComponent function implementation

    has_user_component_str = str()
    for i, component_name in enumerate(components):
        if i == 0:
            has_user_component_str += "\tif (componentID == " + component_hashes[i][0] + ") { return e.HasComponent<" + component_name + ">(); }\n"
        else:
            has_user_component_str += "\telse if (componentID == " + component_hashes[i][0] + ") { return e.HasComponent<" + component_name + ">(); }\n"

    # AddUserComponent function implementation

    add_user_component_str = str()
    for i, component_name in enumerate(components.keys()):
        if i == 0:
            add_user_component_str += "\tif (componentID == " + component_hashes[i][0] + ") { e.AddComponent<" + component_name + ">(); }\n"
        else:
            add_user_component_str += "\telse if (componentID == " + component_hashes[i][0] + ") { e.AddComponent<" + component_name + ">(); }\n"
    
    # RemoveUserComponent function implementation

    remove_user_component_str = str()
    for i, component_name in enumerate(components.keys()):
        if i == 0:
            remove_user_component_str += "\tif (componentID == " + component_hashes[i][0] + ") { e.RemoveComponent<" + component_name + ">(); }\n"
        else:
            remove_user_component_str += "\telse if (componentID == " + component_hashes[i][0] + ") { e.RemoveComponent<" + component_name + ">(); }\n"

    # GetUserComponentVar function implementation

    get_user_component_var_str = str()
    for i, component_name in enumerate(components.keys()):
        if len(components[component_name].variables) <= 0:
            continue

        component = components[component_name]

        if get_user_component_var_str == "":
            get_user_component_var_str += "\tif "
        else:
            get_user_component_var_str += "\telse if "
        
        get_user_component_var_str += "(componentID == " + component_hashes[i][0] + ")\n"
        get_user_component_var_str += "\t{\n"
        get_user_component_var_str += "\t\tauto component = &e.GetComponent<" + component_name + ">();\n\n"

        for j, variable in enumerate(component.variables):
            if j == 0:
                get_user_component_var_str += "\t\tif "
            else:
                get_user_component_var_str += "\t\telse if "
            
            get_user_component_var_str += "(varID == " + component_hashes[i][1][j] + ") { return static_cast<void*>(&component->" + component.variables[j].name + "); }\n"

        get_user_component_var_str += "\t}\n"

    # SetUserComponentVar function implementation

    set_user_component_var_str = str()
    for i, component_name in enumerate(components.keys()):
        if len(components[component_name].variables) <= 0:
            continue

        component = components[component_name]

        if set_user_component_var_str == "":
            set_user_component_var_str += "\tif "
        else:
            set_user_component_var_str += "\telse if "
        
        set_user_component_var_str += "(componentID == " + component_hashes[i][0] + ")\n"
        set_user_component_var_str += "\t{\n"
        set_user_component_var_str += "\t\tauto component = &e.GetComponent<" + component_name + ">();\n\n"

        for j, variable in enumerate(component.variables):
            if j == 0:
                set_user_component_var_str += "\t\tif "
            else:
                set_user_component_var_str += "\t\telse if "
            
            set_user_component_var_str += "(varID == " + component_hashes[i][1][j] + ") { component->" + component.variables[j].name + " = *static_cast<" + var_type_to_cpp(component.variables[j].var_type) + "*>(data); }\n"

        set_user_component_var_str += "\t}\n"

    # File header

    userlib_str = r'''// This file was automatically generated by gen.py at %s.
// Editing the contents of this file manually is not recommended and may result in an application crash.
''' % datetime.datetime.now().strftime("%m/%d/%Y, %H:%M:%S")

    # File body
    
    userlib_str += r'''#pragma once
#include "Omni.h"
#include "domain/Domain.h"
#include "scene/ScriptableEntity.h"
#include "userlib/UserLibraryCore.h"
#include "core/UUID.h"
#include <MurmurHash2/MurmurHash2.h>

%s

using namespace Omni;

using AllUserComponents =
ComponentGroup<
	UUIDComponent,
	NameComponent,
	TransformComponent,
	RelationshipComponent,
	SpriteRendererComponent,
	CameraComponent,
	NativeScriptComponent%s
>;

std::unordered_map<uint64_t, std::string>* g_Domains;
std::unordered_map<uint64_t, std::string>* g_RendererDomains;
std::unordered_map<uint64_t, Omni::ScriptSpecification>* g_Scripts;
std::unordered_map<uint64_t, Omni::ComponentSpecification>* g_Components;

// Hashes

%s
// Hashes

extern "C" __declspec(dllexport) void Init()
{
%s
}

extern "C" __declspec(dllexport) void SetImGuiContext(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
}

extern "C" __declspec(dllexport) void* GetDomainList()
{
    return static_cast<void*>(g_Domains);
}

extern "C" __declspec(dllexport) void* GetRendererDomainList()
{
	return static_cast<void*>(g_RendererDomains);
}

extern "C" __declspec(dllexport) void* GetScriptList()
{
    return static_cast<void*>(g_Scripts);
}

extern "C" __declspec(dllexport) void* GetComponentList()
{
	return static_cast<void*>(g_Components);
}

extern "C" __declspec(dllexport) void* GetDomain(uint64_t domainID)
{
%s
	return nullptr;
}

extern "C" __declspec(dllexport) void* GetRendererDomain(uint64_t domainID)
{
%s
    return nullptr;
}

extern "C" __declspec(dllexport) void* GetScript(uint64_t scriptID)
{
%s
    return nullptr;
}

extern "C" __declspec(dllexport) void* GetScriptVar(uint64_t scriptID, void* script, uint64_t varID)
{
%s
    return nullptr;
}

extern "C" __declspec(dllexport) void* GetScriptSpecs()
{
    std::vector<std::pair<uint64_t, std::vector<Omni::VariableSpecification>*>>* scriptSpecs = new std::vector<std::pair<uint64_t, std::vector<Omni::VariableSpecification>*>>();

%s
    return static_cast<void*>(scriptSpecs);
}

extern "C" __declspec(dllexport) void* GetUserComponent(uint64_t componentID)
{
%s
    return nullptr;
}

extern "C" __declspec(dllexport) bool HasUserComponent(uint64_t entity, void* scene, uint64_t componentID)
{
    Scene* scenePtr = static_cast<Scene*>(scene);
    Omni::Entity e = scenePtr->GetEntity(entity);

%s
    return false;
}

extern "C" __declspec(dllexport) void AddUserComponent(uint64_t entity, void* scene, uint64_t componentID)
{
    Scene* scenePtr = static_cast<Scene*>(scene);
    Omni::Entity e = scenePtr->GetEntity(entity);

%s
}

extern "C" __declspec(dllexport) void RemoveUserComponent(uint64_t entity, void* scene, uint64_t componentID)
{
	Scene* scenePtr = static_cast<Scene*>(scene);
	Omni::Entity e = scenePtr->GetEntity(entity);

%s
}

extern "C" __declspec(dllexport) void* GetUserComponentVar(uint64_t entity, void* scene, uint64_t componentID, uint64_t varID)
{
    Scene* scenePtr = static_cast<Scene*>(scene);
    Omni::Entity e = scenePtr->GetEntity(entity);

%s
    return nullptr;
}

extern "C" __declspec(dllexport) void SetUserComponentVar(uint64_t entity, void* scene, uint64_t componentID, uint64_t varID, void* data)
{
    Scene* scenePtr = static_cast<Scene*>(scene);
    Omni::Entity e = scenePtr->GetEntity(entity);

%s
}

template<typename... Component>
static void CopyUserComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<Omni::UUID, entt::entity>& enttMap)
{
	([&]()
		{
			auto view = src.view<Component>();
			for (auto srcEntity : view)
			{
				entt::entity dstEntity = enttMap.at(src.get<UUIDComponent>(srcEntity).id);

				auto& srcComponent = src.get<Component>(srcEntity);
				dst.emplace_or_replace<Component>(dstEntity, srcComponent);
			}
		}(), ...);
}

template<typename... Component>
static void CopyUserComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<Omni::UUID, entt::entity>& enttMap)
{
	CopyUserComponent<Component...>(dst, src, enttMap);
}

extern "C" __declspec(dllexport) void* CopyUserScene(void* other)
{
	Scene* otherScene = static_cast<Scene*>(other);
	Scene* newScene = new Scene();

	newScene->m_ViewportWidth = otherScene->m_ViewportWidth;
	newScene->m_ViewportHeight = otherScene->m_ViewportHeight;

	// TODO : Copy main camera

	auto& srcSceneRegistry = otherScene->m_Registry;
	auto& dstSceneRegistry = newScene->m_Registry;
	std::unordered_map<Omni::UUID, entt::entity> enttMap;

	// Create entities in new scene
	for (auto e : otherScene->m_SceneOrder)
	{
		Omni::UUID id = e.GetComponent<UUIDComponent>().id;
		const auto& name = e.GetComponent<NameComponent>().Name;
		Entity newEntity = newScene->CreateEntityWithUUID(id, name);
        newEntity.SetActive(e.IsActive());
		enttMap[id] = (entt::entity)newEntity;
	}

	// Copy all components except for UUIDComponent and NameComponent
	CopyUserComponent(AllUserComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

	return static_cast<void*>(newScene);
}
''' % (includes_str, all_user_components_str, hashes_str, init_str, get_domain_str, get_renderer_str, get_script_str, get_script_var_str, get_script_specs_str, get_user_component_str, has_user_component_str, add_user_component_str, remove_user_component_str, get_user_component_var_str, set_user_component_var_str)

    f = open(userlib_path, "w")

    f.write(userlib_str)
    f.close()

    return

def gen():
    print("args %s %s \n" % (sys.argv[1], sys.argv[2]))

    read_src_map()
    scan_src()
    generate_code()

    return
    
start_time = time.time()
gen()
print("gen.py Execution Time: %s seconds" % (time.time() - start_time))
