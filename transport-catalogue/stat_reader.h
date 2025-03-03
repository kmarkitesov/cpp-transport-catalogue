#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace transport::output {

    void ParseAndPrintStat(const catalogue::TransportCatalogue& tansport_catalogue, std::string_view request,
                        std::ostream& output);

    void PrintBusInfo(const catalogue::TransportCatalogue& transport_catalogue, std::string_view bus_name, std::ostream& output);
    void PrintStopInfo(const catalogue::TransportCatalogue& transport_catalogue, std::string_view stop_name, std::ostream& output);

}