#include <common.h>
#include <acpi/aml/parse.h>
#include <linux/dump.h>

#include <config.h>

int main(int argc, char* argv[]){
	std::cout << "Starting Sigma acpi-driver version: " << VERSION_STR << std::endl;

	std::vector<uint8_t> raw_dsdt;
	if(argc == 1){
		// Just dump the local dsdt
		raw_dsdt = linux::dump_table(acpi::tables::dsdt_signature);
	} else {
		for(uint64_t i = 1; i < argc; i++){
			char* str = argv[i];
			if(strlen(str) < 3){
				std::cerr << "Argument error" << std::endl;
				return EXIT_FAILURE;
			}
			if(str[0] == '-'){
				switch (str[1])
				{
				case 'd':
				{
					char* dsdt_file = &str[2];
					raw_dsdt = linux::read_file(std::string(dsdt_file));
					break;
				}
				default:
					std::cerr << "Unknown argument" << std::endl;
					break;
				}
			}
		}
	}

	acpi::tables::table* dsdt = acpi::tables::get_table(raw_dsdt);

	acpi::aml::parser parser(dsdt);

	try {
		parser.parse();
	} catch(std::exception& e){
		std::cerr << "Caught Parser Exception: " << e.what() << std::endl;
		std::cerr << std::endl << "Printing current Abstract Object Tree to stdout" << std::endl;
		parser.get_tree().print();

		return EXIT_FAILURE;
	}

	parser.get_tree().print();

	return EXIT_SUCCESS;
}
