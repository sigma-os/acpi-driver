#ifndef SIGMA_ACPI_SERVER_AML_PARSE
#define SIGMA_ACPI_SERVER_AML_PARSE

#include <common.h>

#include <acpi/tables.h>

#include <acpi/aml/aml_types.h>
#include <acpi/aml/opcodes.h>
#include <acpi/aml/aot_node.h>

#include <types/tree.h>

#define IS_LEADNAMECHAR(a) (((((a) >= 'A') && ((a) <= 'Z')) || ((a) == '_')))
#define IS_DIGITCHAR(a) (((a) >= '0') && ((a) <= '9'))
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

        std::pair<size_t, size_t> parse_pkglength();
        std::pair<std::string, size_t> parse_namestring();
        std::pair<std::string, size_t> parse_namepath();
        std::pair<std::string, size_t> parse_nameseg();

        uint8_t parse_bytedata();
        uint16_t parse_worddata();
        uint32_t parse_dworddata();
        uint64_t parse_qworddata();

        void parse_termlist(size_t bytes_to_parse);
        std::pair<acpi::aml::TermArg, size_t> parse_termarg_integer();
        std::pair<acpi::aml::TermArg, size_t> parse_termarg(acpi::aml::TermArgsTypes type);

        void parse_scopeop();
        void parse_processorop();
        void parse_methodop();
        void parse_opregion();

        uint64_t ip;
        uint8_t* code;
        acpi::tables::sdt_header* code_header;
        tree<acpi::aml::aot_node> abstract_object_tree;
        tree_node<acpi::aml::aot_node>* current_parent;
    };
} // namespace acpi::aml


#endif