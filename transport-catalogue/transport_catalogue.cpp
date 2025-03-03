#include "transport_catalogue.h"
#include <algorithm>
#include <unordered_set>

namespace transport::catalogue {

    void TransportCatalogue::AddStop(const std::string_view& name, const geo::Coordinates& coords){
        stops_.push_back(Stop{std::string(name), coords, {}});
        Stop* stopPtr = &stops_.back();
        stopsByName_[stopPtr->name] = stopPtr;
    }

    void TransportCatalogue::AddBus(const std::string_view& name, const std::vector<std::string_view>& stopNames){
        buses_.push_back(Bus{std::string(name), {}});
        Bus* busPtr = &buses_.back();
        busesByName_[busPtr->name] = busPtr;

        for (const auto& stop : stopNames) {
            Stop* stopPtr = FindStop(stop);
            if (stopPtr) {
                busPtr->stops.push_back(stopPtr);
                stopPtr->buses.insert(busPtr->name);
            }
        }
    }

    const Bus* TransportCatalogue::FindBus(const std::string_view& busName) const {
        auto it = busesByName_.find(busName);
        return it != busesByName_.end() ? it->second : nullptr;
    }

    const Stop* TransportCatalogue::FindStop(const std::string_view& stopName) const {
        auto it = stopsByName_.find(stopName);
        return it != stopsByName_.end() ? it->second : nullptr;
    }

    Stop* TransportCatalogue::FindStop(const std::string_view& stopName) {
        auto it = stopsByName_.find(stopName);
        return it != stopsByName_.end() ? it->second : nullptr;
    }

    BusInfo TransportCatalogue::GetBusInfo(std::string_view& busName) const {
        BusInfo info;

        const Bus* bus = FindBus(busName);
        if(!bus){
            info.stopsCount = 0;
            return info;
        }

        std::unordered_set<const Stop*> uniqueStops;
        for(const auto& stop : bus->stops){
            uniqueStops.insert(stop);
        }

        double length = 0.0;
        for(size_t i = 1; i < bus->stops.size(); ++i){
            length += ComputeDistance(bus->stops[i-1]->coords, bus->stops[i]->coords);
        }

        info.stopsCount = static_cast<int>(bus->stops.size());
        info.uniqueStops = static_cast<int>(uniqueStops.size());
        info.routeLength = length;
        return info;
    }

    std::vector<std::string_view> TransportCatalogue::GetBusesByStop(std::string_view stopName) const{
        const Stop* stop = FindStop(stopName);
        if(!stop || stop->buses.empty()){
            return {};
        }

        std::vector<std::string_view> buses(stop->buses.begin(), stop->buses.end());
        std::sort(buses.begin(), buses.end());
        return buses;
    }

}