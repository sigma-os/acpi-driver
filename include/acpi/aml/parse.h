#ifndef SIGMA_ACPI_SERVER_AML_PARSE
#define SIGMA_ACPI_SERVER_AML_PARSE

#include <common.h>

#include <acpi/tables.h>

#include <acpi/aml/opcodes.h>

namespace acpi::aml
{
    
    class parser {
        public:
        parser(acpi::tables::table* table): ip(0), code(table->data) {}

        void parse();

        private:

        void parse_opcode();
        void parse_ext_opcode();

        size_t parse_pkglength();

        uint64_t ip;
        uint8_t* code;
    };
} // namespace acpi::aml


#endif