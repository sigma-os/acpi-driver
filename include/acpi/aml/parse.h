#ifndef SIGMA_ACPI_SERVER_AML_PARSE
#define SIGMA_ACPI_SERVER_AML_PARSE

#include <common.h>

#include <acpi/tables.h>

#include <acpi/aml/opcodes.h>
#include <acpi/aml/aot_node.h>

#include <types/tree.h>

#define IS_LEADNAMECHAR(a) ((((a) >= 'A' && (a) <= 'Z') || ((a) == '_')))
#define IS_DIGITCHAR(a) ((a) >= '0' && (a) <= '9')
#define IS_NAMECHAR(a) ((IS_LEADNAMECHAR((a))) || (IS_DIGITCHAR((a))))

namespace acpi::aml
{
    
    class parser {
        public:
        parser(acpi::tables::table* table): ip(0), code(table->data), code_header(reinterpret_cast<acpi::tables::sdt_header*>(table)) {}

        void parse();
        tree<acpi::aml::aot_node>& get_tree();

        private:

        uint8_t parse_next_byte();
        uint8_t lookahead_byte(int64_t bytes);

        void parse_opcode();
        void parse_ext_opcode();

        size_t parse_pkglength(size_t& n_bytes_parsed);
        std::string parse_namestring(size_t& n_bytes_parsed);
        std::string parse_namepath(size_t& n_bytes_parsed);

        std::string parse_nameseg(size_t& n_bytes_parsed);

        void parse_termlist(size_t bytes_to_parse);

        void parse_scopeop();

        uint64_t ip;
        uint8_t* code;
        acpi::tables::sdt_header* code_header;
        tree<acpi::aml::aot_node> abstract_object_tree;
    };
} // namespace acpi::aml


#endif