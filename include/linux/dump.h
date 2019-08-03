#ifndef SIGMA_ACPI_SERVER_LINUX_DUMP
#define SIGMA_APCI_SERVER_LINUX_DUMP

#include <common.h>
#include <unistd.h>

namespace linux
{
    std::vector<uint8_t> dump_table(std::string signature);
    std::vector<uint8_t> read_file(std::string filename);
} // namespace linux




#endif
