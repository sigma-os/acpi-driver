#ifndef SIGMA_ACPI_SERVER_AML_AOT_NODE
#define SIGMA_ACPI_SERVER_AML_AOT_NODE

#include <common.h>

namespace acpi::aml
{
    enum class aot_node_types {ROOT, SCOPE, PROCESSOR, METHOD};

    // Abstract Object Tree node, TODO: Add type specific info
    struct aot_node {
        std::string name;
        acpi::aml::aot_node_types type;
        size_t byte_offset;
        size_t pkg_length;
        bool reparse;

        friend std::ostream& operator<<(std::ostream& os, const aot_node& m){
            return os << "[Name: " << m.name << ", Type: " << acpi::aml::aot_node::get_type_string(m.type) << std::hex << ", Offset: 0x" << m.byte_offset << ", PkgLength: 0x" << m.pkg_length << "]";
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
            
            default:
                return "Undefined";
                break;
            }
        }
    };
} // namespace acpi::aml

#endif