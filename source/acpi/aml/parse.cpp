#include <acpi/aml/parse.h>


// PkgLength includes itself but not the opcode and not the extended opcode
size_t acpi::aml::parser::parse_pkglength(size_t& n_bytes_parsed){
    uint8_t PkgLeadByte = this->code[this->ip];

    uint8_t n_bytedata = (PkgLeadByte >> 6) & 0x3;

    n_bytes_parsed = (n_bytedata + 1);

    if(n_bytedata == 0) return PkgLeadByte & 0x3F;
    else if(n_bytedata == 1){
        uint8_t byte1 = this->code[++ip];
        return ((PkgLeadByte & 0x0F) | (byte1 << 4));
    } else if(n_bytedata == 2){
        uint8_t byte1 = this->code[++ip];
        uint8_t byte2 = this->code[++ip];
        return ((PkgLeadByte & 0x0F) | (byte1 << 4) | (byte2 << 12));
    } else if(n_bytedata == 3){
        uint8_t byte1 = this->code[++ip];
        uint8_t byte2 = this->code[++ip];
        uint8_t byte3 = this->code[++ip];
        return ((PkgLeadByte & 0x0F) | (byte1 << 4) | (byte2 << 12) | (byte3 << 20));
    } 
    
    std::cerr << "Impossible to reach";
    return 0;
}

std::string acpi::aml::parser::parse_nameseg(size_t& n_bytes_parsed){
    n_bytes_parsed = 0;

    std::string string;
    uint8_t char1 = this->code[ip];
    if(!IS_LEADNAMECHAR(char1)) throw std::runtime_error("NameSeg first char is not a LeadNameChar");
    string.push_back(char1);
    n_bytes_parsed++;

    uint8_t char2 = this->code[++ip];
    if(!IS_NAMECHAR(char2)) throw std::runtime_error("NameSeg second char is not a NameChar");
    string.push_back(char2);
    n_bytes_parsed++;

    uint8_t char3 = this->code[++ip];
    if(!IS_NAMECHAR(char3)) throw std::runtime_error("NameSeg third char is not a NameChar");
    string.push_back(char3);
    n_bytes_parsed++;

    uint8_t char4 = this->code[++ip];
    if(!IS_NAMECHAR(char4)) throw std::runtime_error("NameSeg fourth char is not a NameChar");
    string.push_back(char4);
    n_bytes_parsed++;

    return string;
}

std::string acpi::aml::parser::parse_namepath(size_t& n_bytes_parsed){
    n_bytes_parsed = 0;
    uint8_t char1 = this->code[ip];
    n_bytes_parsed++;

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
        return std::string();
    
    default:
        size_t name_seg_bytes;
        string.append(this->parse_nameseg(name_seg_bytes)); // Append this
        n_bytes_parsed += name_seg_bytes;
        break;
    }

    return string;
}

std::string acpi::aml::parser::parse_namestring(size_t& n_bytes_parsed){
    n_bytes_parsed = 0;

    uint8_t char1 = this->code[ip];
    n_bytes_parsed++;

    std::string string;

    if(char1 == '^'){
        throw std::runtime_error("PrefixPath is unimplemented, TODO");
    }

    if(char1 == acpi::aml::opcodes::RootChar){
        this->ip++;
        size_t name_path_bytes;
        string.append(this->parse_namepath(name_path_bytes));
        n_bytes_parsed += name_path_bytes;

        return string;
    }

    throw std::runtime_error("Invalid NameString first character");
}


void acpi::aml::parser::parse_scopeop(){
    this->ip++;

    size_t n_pkglength_bytes;
    size_t pkglength = this->parse_pkglength(n_pkglength_bytes);
    pkglength -= n_pkglength_bytes;

    this->ip++;

    size_t n_namestring_bytes;
    std::string s = this->parse_namestring(n_namestring_bytes);
    pkglength -= n_namestring_bytes;
}

void acpi::aml::parser::parse_ext_opcode(){
    this->ip++;
    uint8_t ext_opcode = this->code[this->ip];

    switch (ext_opcode)
    {
    default:
        std::cerr << "Undefined extended opcode: " << std::hex << static_cast<uint64_t>(ext_opcode) << std::endl;
        throw std::runtime_error("Undefined extended opcode");
        break;
    }
}

void acpi::aml::parser::parse_opcode(){
    uint8_t opcode = this->code[this->ip];

    switch (opcode)
    {
    case acpi::aml::opcodes::ExtOpPrefix:
        this->parse_ext_opcode();
        break;

    case acpi::aml::opcodes::ScopeOp:
        this->parse_scopeop();
        break;

    default:
        std::cerr << "Found Undefined opcode: 0x" << std::hex << static_cast<uint64_t>(opcode) << std::endl;
        throw std::runtime_error("Undefined opcode");
        break;
    }
}

void acpi::aml::parser::parse(){
    this->parse_opcode(); // Start recursive descent
    return;
}