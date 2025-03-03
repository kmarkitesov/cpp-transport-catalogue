#pragma once

#include "geo.h"

#include <deque>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace transport::catalogue {

    struct Stop{
        std::string name;
        geo::Coordinates coords;
        std::unordered_set<std::string> buses;
    };

    struct Bus{
        std::string name;
        std::vector<Stop*> stops;
    };

    struct BusInfo{
        int stopsCount = 0;
        int uniqueStops = 0;
        double routeLength = 0.0;
    };

    class TransportCatalogue {
    public:
        void AddStop(const std::string_view& name, const geo::Coordinates& coords);
        void AddBus(const std::string_view& name, const std::vector<std::string_view>& stopNames);
        const Bus* FindBus(const std::string_view& busName) const;
        const Stop* FindStop(const std::string_view& stopName) const;
        Stop* FindStop(const std::string_view& stopName);
        BusInfo GetBusInfo(std::string_view& busName) const;
        std::vector<std::string_view> GetBusesByStop(std::string_view stopName) const;

    private:
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Stop*> stopsByName_;
        std::unordered_map<std::string_view, Bus*> busesByName_;
    };

}