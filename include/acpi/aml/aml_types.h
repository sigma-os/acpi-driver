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

    enum class FieldElementTypes {NamedField, ConnectField}; // The rest of the field types aren't dumped into the tree
    std::string FieldElementTypes_to_string(acpi::aml::FieldElementTypes type);
} // namespace acpi::aml


#endif