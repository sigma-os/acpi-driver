#ifndef SIGMA_ACPI_SERVER_AML_PARSE
#define SIGMA_ACPI_SERVER_AML_PARSE

#include <common.h>

#include <acpi/tables.h>

#include <acpi/aml/opcodes.h>

#define IS_LEADNAMECHAR(a) ((((a) >= 'A' && (a) <= 'Z') || ((a) == '_')))
#define IS_DIGITCHAR(a) ((a) >= '0' && (a) <= '9')
#define IS_NAMECHAR(a) ((IS_LEADNAMECHAR((a))) || (IS_DIGITCHAR((a))))

namespace acpi::aml
{
    
    class parser {
        public:
        parser(acpi::tables::table* table): ip(0), code(table->data) {}

        void parse();

        private:

        void parse_opcode();
        void parse_ext_opcode();

        size_t parse_pkglength(size_t& n_bytes_parsed);
        std::string parse_namestring(size_t& n_bytes_parsed);
        std::string parse_namepath(size_t& n_bytes_parsed);

        std::string parse_nameseg(size_t& n_bytes_parsed);


        void parse_scopeop();

        uint64_t ip;
        uint8_t* code;
    };
} // namespace acpi::aml


#endif