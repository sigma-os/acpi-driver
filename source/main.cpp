#include <common.h>
#include <acpi/aml/parse.h>
#include <linux/dump.h>

#include <config.h>

int main(){
	std::cout << "Starting Sigma acpi-driver version: " << VERSION_STR << std::endl;

	std::vector<uint8_t> raw_dsdt = linux::dump_table(acpi::tables::dsdt_signature);
	acpi::tables::table* dsdt = acpi::tables::get_table(raw_dsdt);

	acpi::aml::parser parser(dsdt);

	try {
		parser.parse();
	} catch(std::exception& e){
		std::cerr << "Caught Parser Exception: " << e.what() << std::endl;
		std::cerr << "Printing current Abstract Object Tree to stdout" << std::endl;
		parser.get_tree().print();

		return EXIT_FAILURE;
	}

	parser.get_tree().print();

	return EXIT_SUCCESS;
}
