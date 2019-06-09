#include <common.h>
#include <acpi/tables.h>
#include <linux/dump.h>

#include <config.h>

int main(){
	std::cout << "Starting Sigma acpi-driver version: " << VERSION_STR << std::endl;

	std::vector<uint8_t> raw_dsdt = linux::dump_table(acpi::tables::dsdt_signature);
	acpi::tables::table* dsdt = acpi::tables::get_table(raw_dsdt);
	return EXIT_SUCCESS;
}
