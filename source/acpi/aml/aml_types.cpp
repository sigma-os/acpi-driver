#include <acpi/aml/aml_types.h>

std::string acpi::aml::TermArgsTypes_to_string(acpi::aml::TermArgsTypes type){
    switch (type)
    {
    case acpi::aml::TermArgsTypes::Integer:
        return "Integer";
        break;

    case acpi::aml::TermArgsTypes::Buffer:
        return "Buffer";
        break;

    case acpi::aml::TermArgsTypes::String:
        return "String";
        break;

    case acpi::aml::TermArgsTypes::ObjectReference:
        return "ObjectReference";
        break;
    
    case acpi::aml::TermArgsTypes::Package:
        return "Package";
        break;
    
    default:
        return "Unknown";
        break;
    }
}

std::string acpi::aml::FieldElementTypes_to_string(acpi::aml::FieldElementTypes type){
    switch (type)
    {
    case acpi::aml::FieldElementTypes::NamedField:
        return "NamedField";
        break;

    case acpi::aml::FieldElementTypes::ConnectField:
        return "ConnectField";
        break;

    default:
        return "Unknown";
        break;
    }
}