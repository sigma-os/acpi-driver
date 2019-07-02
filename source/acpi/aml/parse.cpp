#include <acpi/aml/parse.h>

uint8_t acpi::aml::parser::parse_next_byte(){
    uint8_t ret = this->code[ip];
    this->ip++;
    return ret;
}

uint8_t acpi::aml::parser::lookahead_byte(int64_t bytes){
    return this->code[this->ip + bytes];
}

uint8_t acpi::aml::parser::parse_bytedata(){
    return this->parse_next_byte();
}
uint16_t acpi::aml::parser::parse_worddata(){
    return (this->parse_bytedata() | ((uint16_t)this->parse_bytedata() << 8));
}
uint32_t acpi::aml::parser::parse_dworddata(){
    return (this->parse_worddata() | ((uint32_t)this->parse_worddata() << 16));
}
uint64_t acpi::aml::parser::parse_qworddata(){
    return (this->parse_dworddata() | ((uint64_t)this->parse_dworddata() << 32));
}

// PkgLength includes itself but not the opcode and not the extended opcode
std::pair<size_t, size_t> acpi::aml::parser::parse_pkglength(){
    uint8_t PkgLeadByte = this->parse_next_byte();

    uint8_t n_bytedata = (PkgLeadByte >> 6) & 0x3;

    size_t n_bytes_parsed = (n_bytedata + 1);
    size_t ret;

    switch (n_bytedata)
    {
    case 3:
        {
            uint8_t byte1 = this->parse_next_byte();    
            uint8_t byte2 = this->parse_next_byte();
            uint8_t byte3 = this->parse_next_byte();
            ret = ((PkgLeadByte & 0x0F) | (byte1 << 4) | (byte2 << 12) | (byte3 << 20));
        }
        break;
    case 2:
        {
            uint8_t byte1 = this->parse_next_byte();
            uint8_t byte2 = this->parse_next_byte();
            ret = ((PkgLeadByte & 0x0F) | (byte1 << 4) | (byte2 << 12));
        }
        break;
    case 1:
        {
            uint8_t byte1 = this->parse_next_byte();
            ret = ((PkgLeadByte & 0x0F) | (byte1 << 4));
        }
        break;
    case 0:
        ret = PkgLeadByte & 0x3F;
        break;
    default:
        throw std::runtime_error("Impossible n_bytedata in parse_pkglength");
        break;
    }
    
    return std::pair<size_t, size_t>(n_bytes_parsed, ret);
}

std::pair<std::string, size_t> acpi::aml::parser::parse_nameseg(){
    size_t n_bytes_parsed = 0;

    std::string string;
    uint8_t char1 = this->parse_next_byte();
    if(!IS_LEADNAMECHAR(char1)) throw std::runtime_error("NameSeg first char is not a LeadNameChar");
    string.push_back(char1);
    n_bytes_parsed++;

    uint8_t char2 = this->parse_next_byte();
    if(!IS_NAMECHAR(char2)) throw std::runtime_error("NameSeg second char is not a NameChar");
    string.push_back(char2);
    n_bytes_parsed++;

    uint8_t char3 = this->parse_next_byte();
    if(!IS_NAMECHAR(char3)) throw std::runtime_error("NameSeg third char is not a NameChar");
    string.push_back(char3);
    n_bytes_parsed++;

    uint8_t char4 = this->parse_next_byte();
    if(!IS_NAMECHAR(char4)) throw std::runtime_error("NameSeg fourth char is not a NameChar");
    string.push_back(char4);
    n_bytes_parsed++;

    return std::pair<std::string, size_t>(string, n_bytes_parsed);
}

std::pair<std::string, size_t> acpi::aml::parser::parse_namepath(){
    size_t n_bytes_parsed = 0;
    uint8_t char1 = this->lookahead_byte(0);

    std::string string;

    switch (char1)
    {
    case acpi::aml::opcodes::DualNamePrefix:
        throw std::runtime_error("DualNamePrefix is unimplemented");
        break;
    case acpi::aml::opcodes::MultiNamePrefix:
        throw std::runtime_error("MultiNamePrefix is unimplemented");
        break;

    case 0x0: // NullName
        this->parse_next_byte();
        n_bytes_parsed++;
        return std::pair<std::string, size_t>(std::string(), n_bytes_parsed);
    
    default:
        auto [seg, name_seg_bytes] = this->parse_nameseg();
        string.append(seg); // Append this
        n_bytes_parsed += name_seg_bytes;
        break;
    }

    return std::pair<std::string, size_t>(string, n_bytes_parsed);
}

std::pair<std::string, size_t> acpi::aml::parser::parse_namestring(){
    size_t n_bytes_parsed = 0;

    uint8_t char1 = this->lookahead_byte(0);

    std::string string;

    if(char1 == '^'){ // Probably not the best method but just skip the '^'
        while(this->parse_next_byte() == '^') n_bytes_parsed++;

    } 
    
    if(char1 == acpi::aml::opcodes::RootChar){
        string.push_back(this->parse_next_byte());
        n_bytes_parsed++;
    }
    auto [path, name_path_bytes] = this->parse_namepath();
    string.append(path);
    n_bytes_parsed += name_path_bytes;

    return std::pair<std::string, size_t>(string, n_bytes_parsed);
}


void acpi::aml::parser::parse_scopeop(){
    acpi::aml::aot_node node;

    node.byte_offset = this->ip;
    node.type_specific_data.type = acpi::aml::aot_node_types::SCOPE;
    node.reparse = false;

    auto [n_pkglength_bytes, pkglength] = this->parse_pkglength();
    node.pkg_length = pkglength;
    pkglength -= n_pkglength_bytes;

    auto [s, n_namestring_bytes] = this->parse_namestring();
    node.name = s;
    pkglength -= n_namestring_bytes;

    tree_node<acpi::aml::aot_node>* previous_parent = this->current_parent;
    this->current_parent = this->abstract_object_tree.insert(*(this->current_parent), node);

    this->parse_termlist(pkglength);

    this->current_parent = previous_parent;
}

void acpi::aml::parser::parse_processorop(){
    acpi::aml::aot_node node;
    node.byte_offset = this->ip;
    node.type_specific_data.processor.type = acpi::aml::aot_node_types::PROCESSOR;
    node.reparse = false;

    auto [n_pkglength_bytes, pkglength] = this->parse_pkglength();
    node.pkg_length = pkglength;
    pkglength -= n_pkglength_bytes;

    auto [s, n_namestring_bytes] = this->parse_namestring();
    node.name = s;
    pkglength -= n_namestring_bytes;

    node.type_specific_data.processor.processor_id = this->parse_bytedata();
    node.type_specific_data.processor.pblk_addr = this->parse_dworddata();
    node.type_specific_data.processor.pblk_len = this->parse_bytedata();
    pkglength -= 6;

    tree_node<acpi::aml::aot_node>* previous_parent = this->current_parent;
    this->current_parent = this->abstract_object_tree.insert(*(this->current_parent), node);

    this->parse_termlist(pkglength);

    this->current_parent = previous_parent;
}

void acpi::aml::parser::parse_methodop(){
    acpi::aml::aot_node node;
    node.byte_offset = this->ip;
    node.type_specific_data.method.type = acpi::aml::aot_node_types::METHOD;
    node.reparse = true;

    auto [n_pkglength_bytes, pkglength] = this->parse_pkglength();
    node.pkg_length = pkglength;
    pkglength -= n_pkglength_bytes;

    auto [s, n_namestring_bytes] = this->parse_namestring();
    node.name = s;
    pkglength -= n_namestring_bytes;

    node.type_specific_data.method.method_flags = this->parse_bytedata();
    pkglength -= 1;

    // Don't Parse termlist, it is only done when executing instructions
    this->ip += pkglength;

    this->abstract_object_tree.insert(*(this->current_parent), node);
}

void acpi::aml::parser::parse_opregion(){
    acpi::aml::aot_node node;
    node.byte_offset = this->ip;
    node.type_specific_data.opregion.type = acpi::aml::aot_node_types::OPREGION;
    node.reparse = false;

    node.pkg_length = 0;

    auto [name, name_bytes] = this->parse_namestring();
    node.name = name;
    node.pkg_length += name_bytes;

    uint8_t RegionSpace = this->parse_next_byte();
    node.pkg_length++;
    node.type_specific_data.opregion.space = RegionSpace;

    auto [RegionOffset, regionoffset_size] = this->parse_termarg(acpi::aml::TermArgsTypes::Integer);
    node.pkg_length += regionoffset_size;
    node.type_specific_data.opregion.offset = RegionOffset.termarg.integer.data;

    auto [RegionLen, regionlen_size] = this->parse_termarg(acpi::aml::TermArgsTypes::Integer);
    node.pkg_length += regionlen_size;
    node.type_specific_data.opregion.len = RegionLen.termarg.integer.data;

    this->abstract_object_tree.insert(*(this->current_parent), node);
}

void acpi::aml::parser::parse_termlist(size_t bytes_to_parse){
    if(bytes_to_parse == 0) return;
    uint64_t original_ip = this->ip;

    while(this->ip < (original_ip + bytes_to_parse)){
        this->parse_opcode();
    }
}

std::pair<acpi::aml::TermArg, size_t> acpi::aml::parser::parse_termarg_integer(){
    uint8_t next_byte = this->lookahead_byte(0);
    size_t bytes_parsed = 0;
    acpi::aml::TermArg termarg;
    switch (next_byte)
    {
    case acpi::aml::opcodes::BytePrefix:
        this->parse_next_byte(); // Skip prefix
        termarg.termarg.integer.type = acpi::aml::TermArgsTypes::Integer;
        termarg.termarg.integer.data = this->parse_bytedata();
        bytes_parsed += 2;
        break;

    case acpi::aml::opcodes::WordPrefix:
        this->parse_next_byte(); // Skip prefix
        termarg.termarg.integer.type = acpi::aml::TermArgsTypes::Integer;
        termarg.termarg.integer.data = this->parse_worddata();
        bytes_parsed += 3;
        break;

    case acpi::aml::opcodes::DWordPrefix:
        this->parse_next_byte(); // Skip prefix
        termarg.termarg.integer.type = acpi::aml::TermArgsTypes::Integer;
        termarg.termarg.integer.data = this->parse_dworddata();
        bytes_parsed += 5;
        break;

    case acpi::aml::opcodes::QWordPrefix:
        this->parse_next_byte(); // Skip prefix
        termarg.termarg.integer.type = acpi::aml::TermArgsTypes::Integer;
        termarg.termarg.integer.data = this->parse_qworddata();
        bytes_parsed += 9;
        break;
    
    default:
        std::cerr << "Unknown TermArg next bytestream value: " << static_cast<uint64_t>(next_byte) << std::endl;
        throw std::runtime_error("Unknown TermArg bytestream value");
        break;
    }
    return std::pair<acpi::aml::TermArg, size_t>(termarg, bytes_parsed);
}

std::pair<acpi::aml::TermArg, size_t> acpi::aml::parser::parse_termarg(acpi::aml::TermArgsTypes type){
    switch (type)
    {
    case acpi::aml::TermArgsTypes::Integer:
        return this->parse_termarg_integer();
        break;
    
    default:
        std::ostringstream stream;
        stream << "Unimplemented TermArg type: ";
        stream << acpi::aml::TermArgsTypes_to_string(type);
        throw std::runtime_error(stream.str());
        break;
    }
}

void acpi::aml::parser::parse_ext_opcode(){
    uint8_t ext_opcode = this->parse_next_byte();

    switch (ext_opcode)
    {
    case acpi::aml::opcodes::ext_ProcessorOp:
        this->parse_processorop();
        break;

    case acpi::aml::opcodes::ext_OpRegionOp:
        this->parse_opregion();
        break;

    default:
        std::cerr << "Undefined extended opcode: 0x" << std::hex << static_cast<uint64_t>(ext_opcode) << std::endl;
        throw std::runtime_error("Undefined extended opcode");
        break;
    }
}

void acpi::aml::parser::parse_opcode(){
    uint8_t opcode = this->parse_next_byte();

    switch (opcode)
    {
    case acpi::aml::opcodes::ExtOpPrefix:
        this->parse_ext_opcode();
        break;

    case acpi::aml::opcodes::ScopeOp:
        this->parse_scopeop();
        break;

    case acpi::aml::opcodes::MethodOp:
        this->parse_methodop();
        break;

    default:
        std::cerr << "Found Undefined opcode: 0x" << std::hex << static_cast<uint64_t>(opcode) << std::endl;
        throw std::runtime_error("Undefined opcode");
        break;
    }
}

void acpi::aml::parser::parse(){
    acpi::aml::aot_node& root = this->abstract_object_tree.get_root()->item;

    root.name = "[Root]";
    root.type_specific_data.type = acpi::aml::aot_node_types::ROOT;
    root.byte_offset = 0;
    root.pkg_length = this->code_header->length - sizeof(acpi::tables::sdt_header);

    current_parent = this->abstract_object_tree.get_root();

    this->parse_termlist(this->code_header->length - sizeof(acpi::tables::sdt_header)); // Start recursive descent
    return;
}

tree<acpi::aml::aot_node>& acpi::aml::parser::get_tree(){
    return this->abstract_object_tree;
}