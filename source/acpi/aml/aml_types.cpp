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