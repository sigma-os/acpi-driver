#include <acpi/aml/aml_types.h>

std::string acpi::aml::object_types_to_string(acpi::aml::object_types type){
    switch (type)
    {
    case acpi::aml::object_types::Integer:
        return "Integer";
        break;

    case acpi::aml::object_types::Buffer:
        return "Buffer";
        break;

    case acpi::aml::object_types::String:
        return "String";
        break;

    case acpi::aml::object_types::ObjectReference:
        return "ObjectReference";
        break;
    
    case acpi::aml::object_types::Package:
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