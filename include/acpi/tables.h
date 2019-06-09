#ifndef SIGMA_ACPI_SERVER_ACPI_TABLES
#define SIGMA_ACPI_SERVER_ACPI_TABLES

#include <common.h>

namespace acpi::tables
{
    constexpr const char* dsdt_signature = "dsdt";
    constexpr const char* ssdt_signature = "SSDT";

    struct sdt_header
    {
        char signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        char oem_id[6];
        char oem_tableid[8];
        uint32_t oem_revision;
        uint32_t creator_id;
        uint32_t creator_revision;
    } __attribute__((packed));

    struct table {
        acpi::tables::sdt_header header;
        uint8_t data[1];
    } __attribute__((packed));

    acpi::tables::table* get_table(std::vector<uint8_t>& data);
} // namespace name



#endif // !SIGMA_ACPI_SERVER_ACPI_TABLES