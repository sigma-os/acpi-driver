#ifndef SIGMA_ACPI_SERVER_AML_AOT_NODE
#define SIGMA_ACPI_SERVER_AML_AOT_NODE

#include <common.h>

namespace acpi::aml
{
    enum class aot_node_types {ROOT, SCOPE, PROCESSOR, METHOD, OPREGION};

    struct aot_node {
        std::string name;
        size_t byte_offset;
        size_t pkg_length;
        bool reparse;


        union _type_specific_data
        {
            acpi::aml::aot_node_types type;
            struct _processor {
                acpi::aml::aot_node_types type;
                uint8_t processor_id;
                uint32_t pblk_addr;
                uint8_t pblk_len;
            };
            _processor processor;
            struct _method {
                acpi::aml::aot_node_types type;
                uint8_t method_flags;
            };
            _method method;
            struct _opregion {
                acpi::aml::aot_node_types type;
                uint8_t space;
                uint64_t offset; // TermArg => Integer
                uint64_t len; // TermArg => Integer
            };
            _opregion opregion;
        };
        _type_specific_data type_specific_data;
        

        friend std::ostream& operator<<(std::ostream& os, const aot_node& m){
            return os << "[Name: " << m.name << ", Type: " << acpi::aml::aot_node::get_type_string(m.type_specific_data.type) << std::hex << ", Offset: 0x" << m.byte_offset << ", PkgLength: 0x" << m.pkg_length << ", Type specific data: " << acpi::aml::aot_node::get_type_data(m.type_specific_data) << "]";
        }

        static std::string get_type_string(acpi::aml::aot_node_types type){
            switch (type)
            {
            case acpi::aml::aot_node_types::SCOPE:
                return "Scope";
                break;
            
            case acpi::aml::aot_node_types::PROCESSOR:
                return "Processor";
                break;

            case acpi::aml::aot_node_types::ROOT:
                return "Root";
                break;

            case acpi::aml::aot_node_types::METHOD:
                return "Method";
                break;
            
            case acpi::aml::aot_node_types::OPREGION:
                return "OpRegion";
                break;
            
            default:
                return "Undefined";
                break;
            }
        }

        static std::string get_type_data(acpi::aml::aot_node::_type_specific_data type){
            std::ostringstream stream;
            switch (type.type)
            {
            case acpi::aml::aot_node_types::PROCESSOR:
                stream << "[Processor: ProcID: 0x";
                stream << std::hex << static_cast<uint64_t>(type.processor.processor_id) << ", PblkAddr: 0x";
                stream << std::hex << type.processor.pblk_addr << ", PblkLen: 0x";
                stream << std::hex << static_cast<uint64_t>(type.processor.pblk_len) << "]";
                break;
            case acpi::aml::aot_node_types::METHOD:
                stream << "[Method: Flags: 0x";
                stream << std::hex << static_cast<uint64_t>(type.method.method_flags);
                stream << "]";
                break;
            case acpi::aml::aot_node_types::OPREGION:
                stream << "[OpRegion: Space: 0x";
                stream << std::hex << static_cast<uint64_t>(type.opregion.type) << ", Offset: 0x";
                stream << std::hex << type.opregion.offset << ", Size: 0x";
                stream << std::hex << type.opregion.len << "]";
                break;
            default:
                stream << "None";
                break;
            }
            return stream.str();
        }
    };
} // namespace acpi::aml

#endif