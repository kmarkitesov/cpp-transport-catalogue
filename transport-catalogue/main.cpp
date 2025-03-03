#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    transport::catalogue::TransportCatalogue catalogue;

    transport::input::ReadBaseRequests(std::cin, catalogue);

    transport::output::ReadStatRequests(std::cin, std::cout, catalogue);
}