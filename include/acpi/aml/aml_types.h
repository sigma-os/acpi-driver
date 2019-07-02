#ifndef SIGMA_ACPI_SERVER_AML_TYPES
#define SIGMA_ACPI_SERVER_AML_TYPES

#include <common.h>

namespace acpi::aml
{
    enum class TermArgsTypes {Integer, Buffer, String, ObjectReference, Package};
    std::string TermArgsTypes_to_string(acpi::aml::TermArgsTypes type);

    struct TermArg {
        union _type {
            acpi::aml::TermArgsTypes type;
            struct _integer {
                acpi::aml::TermArgsTypes type;
                uint64_t data;
            };
            _integer integer;
        };
        _type termarg;
    };
} // namespace acpi::aml


#endif