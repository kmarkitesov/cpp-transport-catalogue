#include "stat_reader.h"
#include <iomanip>
#include <string>

namespace transport::output {

    void ParseAndPrintStat(const catalogue::TransportCatalogue& tansport_catalogue, std::string_view request,
                        std::ostream& output) {
        size_t space_pos = request.find(' ');
        if (space_pos == std::string_view::npos) {
            output << "Invalid request" << std::endl;
            return;
        }
        std::string_view type = request.substr(0, space_pos);
        std::string_view name = request.substr(space_pos + 1);

        if (type == "Bus") {
            PrintBusInfo(tansport_catalogue, name, output);
        } else if (type == "Stop") {
            PrintStopInfo(tansport_catalogue, name, output);
        } else {
            output << "Invalid request" << std::endl;
        }
    }

    void PrintBusInfo(const catalogue::TransportCatalogue& transport_catalogue, std::string_view bus_name, std::ostream& output) {
        auto info = transport_catalogue.GetBusInfo(bus_name);
        if (info.stopsCount == 0) {
            output << "Bus " << bus_name << ": not found" << std::endl;
            return;
        }

        double curvature = info.routeLength  / info.geoDistance;
        output << "Bus " << bus_name << ": " 
            << info.stopsCount << " stops on route, " 
            << info.uniqueStops << " unique stops, " 
            << info.routeLength << " route length, "
            << std::fixed << std::setprecision(5) << curvature << " curvature" << std::endl;
    }

    void PrintStopInfo(const catalogue::TransportCatalogue& transport_catalogue, std::string_view stop_name, std::ostream& output) {
        const transport::catalogue::Stop* stop = transport_catalogue.FindStop(stop_name);
        if (!stop) {
            output << "Stop " << stop_name << ": not found" << std::endl;
            return;
        }
        
        const auto& buses = transport_catalogue.GetBusesByStop(stop_name);
        if (buses.empty()) {
            output << "Stop " << stop_name << ": no buses" << std::endl;
            return;
        }

        output << "Stop " << stop_name << ": buses";
        for (const auto& bus : buses) {
            output << " " << bus;
        }
        output << std::endl;
    }

    void ReadStatRequests(std::istream& input, std::ostream& output, const catalogue::TransportCatalogue& catalogue) {
        int stat_request_count;
        input >> stat_request_count >> std::ws;

        for (int i = 0; i < stat_request_count; ++i) {
            std::string line;
            std::getline(input, line);
            ParseAndPrintStat(catalogue, line, output);
        }
    }
}