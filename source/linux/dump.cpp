#include <linux/dump.h>




std::vector<uint8_t> linux::dump_table(std::string signature){
    if(geteuid() != 0){
        std::cerr << "Can't dump ACPI Tables without root" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::stringstream command;
    command << "acpidump -n ";
    command << signature;
    command << " -b";

    system(command.str().c_str());

    std::stringstream filename;
    filename << signature;
    filename << ".dat";

    std::ifstream file(filename.str().c_str(), std::ios::binary);

    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}