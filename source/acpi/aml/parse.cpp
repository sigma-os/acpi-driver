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

// If the node doesnt exist, create it and return it, caller should fill out the info
tree_node<acpi::aml::aot_node>* acpi::aml::parser::resolve_path(std::string path){
    auto namesegs = std::vector<std::pair<std::string, int>>();
                                    //    Actual Seg,  Type
                                    //                 0 = Root char '\'
                                    //                 1 = Parent prefix '^'
                                    //                 2 = Actual segment

    for(uint64_t i = 0; i < path.length();){
        char c = path[i];

        switch(c){
        case '\\': // Root char
            if(i != 0){
                std::cerr << "Non first char root char in: resolve_path" << std::endl;
                throw std::runtime_error("Unexpected char in resolve_path");
            }
            namesegs.push_back({path.substr(i, 1), 0});
            i++;
            break;
        case '^':
            namesegs.push_back({path.substr(i, 1), 1});
            i++;
            break;
        
        case '.': // Name Separator
            i++; // Skip segment separator
            break;
        default: // Just a local NameSeg
            namesegs.push_back({path.substr(i, 4), 2}); // NameSeg is four characters
            i += 4;
            break;
        }
    }

    auto* node = this->current_parent;

    for(auto& it : namesegs){
        auto [seg, type] = it;
        switch (type)
        {
        case 0: // Root item, so current node is root
            node = this->get_tree().get_root();
            break;

        case 1: // Parent Prefix, so go 1 up in the chain
            if(node->parent != nullptr) node = node->parent;
            else throw std::runtime_error("Node doesn't have parent");
            break;

        case 2: // Actual segment, this is where the magic happens
        {   
            bool found = false;
            for(auto& child : node->children){
                if(child->item.name == seg){
                    node = child;
                    found = true;
                    break;
                }
            }
            if(!found){
                // The object doesn't exist i guess, create a new node
                return this->get_tree().insert(*this->current_parent, {});
            }
            break;
        }
        default:
            std::cerr << "Unknown resolve_path path type: " << std::hex << type << std::endl;
            throw std::runtime_error("Unknown resolve_path path type");
            break;
        }
    }

    return node;
}

// TODO: If the name is an existing object parse into that
void acpi::aml::parser::parse_scopeop(){
    uint64_t byte_offset = this->ip;
    auto [n_pkglength_bytes, pkglength] = this->parse_pkglength();
    uint64_t pkg_length = pkglength;
    pkglength -= n_pkglength_bytes;

    auto [s, n_namestring_bytes] = this->parse_namestring();
    std::string name = s;
    pkglength -= n_namestring_bytes;
    

    auto* node = resolve_path(name);
    node->item.byte_offset = byte_offset;
    node->item.pkg_length = pkg_length;
    node->item.name = name;
    node->item.type_specific_data.type = acpi::aml::aot_node_types::SCOPE;
    node->item.reparse = false;

    tree_node<acpi::aml::aot_node>* previous_parent = this->current_parent;
    this->current_parent = node;


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

void acpi::aml::parser::parse_opregionop(){
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

    auto [RegionOffset, regionoffset_size] = this->parse_termarg();
    node.pkg_length += regionoffset_size;
    node.type_specific_data.opregion.offset = RegionOffset.data.integer.num;

    auto [RegionLen, regionlen_size] = this->parse_termarg();
    node.pkg_length += regionlen_size;
    node.type_specific_data.opregion.len = RegionLen.data.integer.num;

    this->abstract_object_tree.insert(*(this->current_parent), node);
}

void acpi::aml::parser::parse_fieldop(){
    acpi::aml::aot_node node;
    node.byte_offset = this->ip;
    node.type_specific_data.processor.type = acpi::aml::aot_node_types::FIELD;
    node.reparse = false;

    auto [n_pkglength_bytes, pkglength] = this->parse_pkglength();
    node.pkg_length = pkglength;
    pkglength -= n_pkglength_bytes;

    auto [s, n_namestring_bytes] = this->parse_namestring();
    node.name = s;
    pkglength -= n_namestring_bytes;

    node.type_specific_data.field.field_flags = this->parse_bytedata();
    pkglength--;


    tree_node<acpi::aml::aot_node>* previous_parent = this->current_parent;
    this->current_parent = this->abstract_object_tree.insert(*(this->current_parent), node);

    uint8_t flags = node.type_specific_data.field.field_flags;
    size_t end_ip = this->ip + pkglength;
    while(this->ip < end_ip){
        uint8_t next_byte = this->lookahead_byte(0);
        switch (next_byte)
        {
        case 0x0: // Reserved Field
            this->parse_next_byte();
            this->parse_pkglength();
            break;

        case 1: // Access field
            this->parse_next_byte();
            flags = this->parse_bytedata();
            this->parse_next_byte();
        case 2:
            throw std::runtime_error("Error ConnectField unimplemented");
            break;

        default:
            {
                acpi::aml::aot_node node;
                node.byte_offset = this->ip;
                node.type_specific_data.processor.type = acpi::aml::aot_node_types::FIELD_ELEMENT;
                node.reparse = false;

                auto [name, name_length] = this->parse_nameseg();
                node.name = name;

                auto [n_pkglength_bytes, pkglength] = this->parse_pkglength();
                node.pkg_length = pkglength;
                node.type_specific_data.field_element.length = pkglength;
                node.type_specific_data.field_element.flags = flags;
                node.type_specific_data.field_element.field_element_type = acpi::aml::FieldElementTypes::NamedField;

                this->abstract_object_tree.insert(*(this->current_parent), node);
            }
            break;  
        }
    }

    this->current_parent = previous_parent;
}

void acpi::aml::parser::parse_deviceop(){
    acpi::aml::aot_node node;
    node.byte_offset = this->ip;
    node.type_specific_data.type = acpi::aml::aot_node_types::DEVICE;
    node.reparse = true;

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

void acpi::aml::parser::parse_nameop(){
    uint64_t byte_offset = this->ip;
    uint64_t pkg_length = 0;

    auto [name, name_bytes] = this->parse_namestring();
    pkg_length += name_bytes;

    auto* node = this->resolve_path(name);
    node->item.byte_offset = byte_offset;
    node->item.name = name;
    node->item.type_specific_data.name.type = acpi::aml::aot_node_types::NAME;
    node->item.reparse = false;

    auto [object, size] = this->parse_data_ref_object();
    pkg_length += size;

    node->item.type_specific_data.name.object = object;
    node->item.pkg_length = pkg_length;
}

void acpi::aml::parser::parse_termlist(size_t bytes_to_parse){
    if(bytes_to_parse == 0) return;
    uint64_t original_ip = this->ip;

    while(this->ip < (original_ip + bytes_to_parse)){
        this->parse_opcode();
    }
}

// ByteConst := BytePrefix ByteData
std::pair<uint64_t, size_t> acpi::aml::parser::parse_byteconst(){
    if(this->parse_next_byte() != acpi::aml::opcodes::BytePrefix) throw std::runtime_error("First byte of ByteConst is not BytePrefix");
    return std::pair<uint64_t, size_t>(this->parse_bytedata(), 2);
}

// WordConst := WordPrefix WordData
std::pair<uint64_t, size_t> acpi::aml::parser::parse_wordconst(){
    if(this->parse_next_byte() != acpi::aml::opcodes::WordPrefix) throw std::runtime_error("First byte of WordConst is not WordPrefix");
    return std::pair<uint64_t, size_t>(this->parse_worddata(), 3);
}

// DWordConst := DWordPrefix DWordData
std::pair<uint64_t, size_t> acpi::aml::parser::parse_dwordconst(){
    if(this->parse_next_byte() != acpi::aml::opcodes::DWordPrefix) throw std::runtime_error("First byte of DWordConst is not DWordPrefix");
    return std::pair<uint64_t, size_t>(this->parse_dworddata(), 5);
}

// QWordConst := QWordPrefix QWordData
std::pair<uint64_t, size_t> acpi::aml::parser::parse_qwordconst(){
    if(this->parse_next_byte() != acpi::aml::opcodes::QWordPrefix) throw std::runtime_error("First byte of QWordConst is not QWordPrefix");
    return std::pair<uint64_t, size_t>(this->parse_qworddata(), 9);
}

// BufferOp := BufferOp PkgLength BufferSize ByteList
std::tuple<uint8_t*, size_t, size_t> acpi::aml::parser::parse_bufferop(){
    if(this->parse_next_byte() != acpi::aml::opcodes::BufferOp) throw std::runtime_error("First byte of DefBuffer is not BufferOp");
    
    auto [n_pkglength_bytes, pkglength] = this->parse_pkglength();;

    auto [buffer_size, buffer_size_len] = this->parse_termarg(); // Evaluates to Integer

    size_t len = buffer_size.data.integer.num;
    auto* ptr = new uint8_t[len];

    for(uint64_t i = 0; i < len; i++) ptr[i] = this->parse_next_byte();

    return std::tuple<uint8_t*, size_t, size_t>(ptr, len, pkglength);
}

std::pair<uint64_t, size_t> acpi::aml::parser::parse_zeroop(){
    return std::pair(0, 1);
}

std::pair<uint64_t, size_t> acpi::aml::parser::parse_oneop(){
    return std::pair(1, 1);
}

std::pair<uint64_t, size_t> acpi::aml::parser::parse_onesop(){
    return std::pair(~(uint64_t)(0), 1);
}

// ComputationalData := ByteConst | WordConst | DWordConst | QWordConst | String | ConstObj | RevisionOp | DefBuffer
std::pair<acpi::aml::object, size_t> acpi::aml::parser::parse_computational_data(){
    acpi::aml::object object;
    size_t size_parsed = 0;
    uint8_t next_byte = this->lookahead_byte(0);
    switch (next_byte)
    {
    case acpi::aml::opcodes::BytePrefix:
        {
            auto [value, size] = this->parse_byteconst();
            object.type = acpi::aml::object_types::Integer;
            object.length = size;
            object.data.integer.num = value;
            size_parsed += size;
        }   
        break;
    
    case acpi::aml::opcodes::WordPrefix:
        {
            auto [value, size] = this->parse_wordconst();
            object.type = acpi::aml::object_types::Integer;
            object.length = size;
            object.data.integer.num = value;
            size_parsed += size;
        }
        break;

    case acpi::aml::opcodes::DWordPrefix:
        {
            auto [value, size] = this->parse_dwordconst();
            object.type = acpi::aml::object_types::Integer;
            object.length = size;
            object.data.integer.num = value;
            size_parsed += size;
        }
        break;

    case acpi::aml::opcodes::QWordPrefix:
        {
            auto [value, size] = this->parse_qwordconst();
            object.type = acpi::aml::object_types::Integer;
            object.length = size;
            object.data.integer.num = value;
            size_parsed += size;
        }
        break;
    case acpi::aml::opcodes::BufferOp:
        {
            auto [ptr, len, size] = this->parse_bufferop();
            object.type = acpi::aml::object_types::Buffer;
            object.length = size;
            object.data.buffer.ptr_len = len;
            object.data.buffer.ptr = ptr;
            size_parsed += size;
        }
        break;
    
    case acpi::aml::opcodes::ZeroOp:
        [[fallthrough]];
    case acpi::aml::opcodes::OneOp:
        [[fallthrough]];
    case acpi::aml::opcodes::OnesOp:
        // Its a ConstObj
        {
            auto [var, size] = this->parse_const_object();
            object = var; // Safe since its an integer
            size_parsed += size;
        }
        break;

    //case acpi::aml::opcodes::StringPrefix: // String := StringPrefix AsciiCharList NullChar
    //    break; // TODO

    default:
        std::cerr << "Unknown ComputeData next bytestream value: 0x" << std::hex << static_cast<uint64_t>(next_byte) << std::endl;
        throw std::runtime_error("Unknown ComputeData bytestream value");
        break;
    }

    return std::pair<acpi::aml::object, size_t>(object, size_parsed);
}

// ConstObj := ZeroOp | OneOp | OnesOp
std::pair<acpi::aml::object, size_t> acpi::aml::parser::parse_const_object(){
    acpi::aml::object object;
    size_t size_parsed = 0;
    uint8_t next_byte = this->parse_next_byte();
    switch (next_byte)
    {
    case acpi::aml::opcodes::ZeroOp:
        {
            auto [val, len] = this->parse_zeroop();
            object.type = acpi::aml::object_types::Integer;
            object.length = len;
            object.data.integer.num = val;
        }
        break;
    case acpi::aml::opcodes::OneOp:
        {
            auto [val, len] = this->parse_oneop();
            object.type = acpi::aml::object_types::Integer;
            object.length = len;
            object.data.integer.num = val;
        }
        break;
    case acpi::aml::opcodes::OnesOp:
        {
            auto [val, len] = this->parse_onesop();
            object.type = acpi::aml::object_types::Integer;
            object.length = len;
            object.data.integer.num = val;
        }
        break;
    default:
        std::cerr << "Unknown ComputeData next bytestream value: 0x" << std::hex << static_cast<uint64_t>(next_byte) << std::endl;
        throw std::runtime_error("Unknown ComputeData bytestream value");
        break;
    }

    return std::pair<acpi::aml::object, size_t>(object, size_parsed);
}

// DataObject := ComputationalData | DefPackage | DefVarPackage
std::pair<acpi::aml::object, size_t> acpi::aml::parser::parse_data_object(){
    acpi::aml::object object;
    size_t size_parsed = 0;

    uint8_t next_byte = this->lookahead_byte(0);
    switch (next_byte)
    {
    case acpi::aml::opcodes::PackageOp:
        throw std::runtime_error("DataObject: Have not implemented DefPackage");
        break;
    
    case acpi::aml::opcodes::VarPackageOp:
        throw std::runtime_error("DataObject: Have not implemented DefVarPackage");
        break;
    
    // No Package or VarPackage well must be a ComputationalData then
    default:
        auto [value, size] = this->parse_computational_data();
        object = value;
        size_parsed = size;
        break;
    }

    return std::pair<acpi::aml::object, size_t>(object, size_parsed);
}

// DataRefObject := DataObject | ObjectReference | DDBHandle
std::pair<acpi::aml::object, size_t> acpi::aml::parser::parse_data_ref_object(){
    acpi::aml::object object;
    size_t size_parsed = 0;

    uint8_t next_byte = this->lookahead_byte(0);
    switch (next_byte)
    {
    // I don't know what ObjectReference or DDBHandle are so just assume that is is an DataObject
    
    default:
        auto [value, size] = this->parse_data_object();
        object = value;
        size_parsed = size;
        break;
    }


    return std::pair<acpi::aml::object, size_t>(object, size_parsed);
}

// TermArg := Type2Opcode | DataObject | ArgObject | LocalObject
std::pair<acpi::aml::object, size_t> acpi::aml::parser::parse_termarg(){
    acpi::aml::object object;
    size_t size_parsed = 0;

    uint8_t next_byte = this->lookahead_byte(0);

    switch (next_byte)
    {
    // Can't implement the other ones right now so assume that it's a DataObject
    
    default:
        auto [value, size] = this->parse_data_object();
        object = value;
        size_parsed = size;
        break;
    }

    return std::pair<acpi::aml::object, size_t>(object, size_parsed);
}

void acpi::aml::parser::parse_ext_opcode(){
    uint8_t ext_opcode = this->parse_next_byte();

    switch (ext_opcode)
    {
    case acpi::aml::opcodes::ext_ProcessorOp:
        this->parse_processorop();
        break;

    case acpi::aml::opcodes::ext_OpRegionOp:
        this->parse_opregionop();
        break;

    case acpi::aml::opcodes::ext_FieldOp:
        this->parse_fieldop();
        break;

    case acpi::aml::opcodes::ext_DeviceOp:
        this->parse_deviceop();
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

    case acpi::aml::opcodes::NameOp:
        this->parse_nameop();
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