#ifndef SIGMA_ACPI_SERVER_AML_PARSE
#define SIGMA_ACPI_SERVER_AML_PARSE

#include <common.h>

#include <acpi/tables.h>

namespace acpi::aml
{
    
    class parser {
        public:
        parser(acpi::tables::table* table): aml_table(table) {}

        void parse();

        private:
        acpi::tables::table* aml_table;
    };
} // namespace acpi::aml


#endif