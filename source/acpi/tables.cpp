#include <acpi/tables.h>

acpi::tables::table* acpi::tables::get_table(std::vector<uint8_t>& data){
    acpi::tables::table* table = new (data.data()) acpi::tables::table;

    std::string signature(table->header.signature);
    std::string oem_id(table->header.oem_id);

    std::cout << "Found new table: Signature: " << signature.substr(0, 4) << ", OEM: " << oem_id.substr(0, 6) << std::endl;

    uint8_t sum = 0;

    for(auto check : data) sum += check;

    if(sum != 0) std::cerr << "Error: Table checksum mismatch" << std::endl;

    return table;
}