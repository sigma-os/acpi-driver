#ifndef SIGMA_ACPI_SERVER_AML_AOT_NODE
#define SIGMA_ACPI_SERVER_AML_AOT_NODE

#include <common.h>

namespace acpi::aml
{
    // Abstract Object Tree node
    struct aot_node {
        std::string name;
        std::string type;
        size_t byte_offset;
        size_t pkg_length;

        friend std::ostream& operator<<(std::ostream& os, const aot_node& m){
            return os << "[Name: " << m.name << ", Type: " << m.type << std::hex << ", Offset: 0x" << m.byte_offset << ", PkgLength: 0x" << m.pkg_length << "]";
        }
    };
} // namespace acpi::aml

#endif