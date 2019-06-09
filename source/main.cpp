#include <common.h>
#include <acpi/tables.h>
#include <linux/dump.h>

#include <config.h>

int main(){
	std::cout << "Starting Sigma acpi-driver version: " << VERSION_STR << std::endl;


	std::vector<uint8_t> buffer = linux::dump_table(acpi::tables::dsdt_signature);

	for(auto a : buffer) std::cout << a << std::endl;
	return EXIT_SUCCESS;
}
