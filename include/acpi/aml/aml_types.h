#ifndef SIGMA_ACPI_SERVER_AML_TYPES
#define SIGMA_ACPI_SERVER_AML_TYPES

#include <common.h>

namespace acpi::aml
{
    enum class object_types {Integer, Buffer, String, ObjectReference, Package};
    std::string object_types_to_string(acpi::aml::object_types type);

    struct object {
        acpi::aml::object_types type;
        size_t length;
        union _data
        {
            struct _integer {
                uint64_t num;
            };
            _integer integer;
            struct _buffer {
                uint8_t* ptr;
                size_t ptr_len;
            };
            _buffer buffer;
        };
        _data data;

        friend std::ostream& operator<<(std::ostream& os, const object& m){
            return os << "[Object: Type: " << acpi::aml::object_types_to_string(m.type) << ", Length: 0x" << std::hex << m.length << ", Value: " << acpi::aml::object::data_to_string(m) << "]";
        }

        static std::string data_to_string(object ob){
            std::ostringstream stream;
            switch (ob.type)
            {
            case acpi::aml::object_types::Integer:
                stream << ob.data.integer.num;
                break;
            case acpi::aml::object_types::Buffer:
                stream << "Buffer";
                break;
            
            default:
                stream << "None";
                break;
            }

            return stream.str();
        }
    };

    enum class FieldElementTypes {NamedField, ConnectField}; // The rest of the field types aren't dumped into the tree
    std::string FieldElementTypes_to_string(acpi::aml::FieldElementTypes type);
} // namespace acpi::aml


#endif