#include <acpi/aml/parse.h>

size_t acpi::aml::parser::parse_pkglength(){
    uint8_t PkgLeadByte = this->code[this->ip];
    uint8_t n_bytedata = PkgLeadByte & 0xC0;

    if(n_bytedata == 0) return PkgLeadByte & 0x3F;
    else if(n_bytedata == 1) return ((PkgLeadByte & 0x0F) | (this->code[ip] << 4));
    else if(n_bytedata == 2) return ((PkgLeadByte & 0x0F) | (this->code[ip] << 4) | (this->code[ip] << 12));
    else if(n_bytedata == 3) return ((PkgLeadByte & 0x0F) | (this->code[ip] << 4) | (this->code[ip] << 12) | (this->code[ip] << 20));
    
    std::cerr << "Impossible to reach";
    return 0;
}

void acpi::aml::parser::parse_ext_opcode(){
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
        this->ip++;
        this->parse_ext_opcode();
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