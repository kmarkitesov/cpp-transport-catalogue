#pragma once

#include "geo.h"

#include <deque>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>

namespace transport::catalogue {

    struct Stop{
        std::string name;
        geo::Coordinates coords;
        std::set<std::string> buses;
    };

    struct Bus{
        std::string name;
        std::vector<Stop*> stops;
    };

    struct BusInfo{
        int stopsCount = 0;
        int uniqueStops = 0;
        double routeLength = 0.0;
        double geoDistance = 0.0;
    };

    class TransportCatalogue {
    public:
        void AddStop(const std::string_view& name, const geo::Coordinates& coords);
        void AddBus(const std::string_view& name, const std::vector<std::string_view>& stopNames);
        const Bus* FindBus(const std::string_view& busName) const;
        const Stop* FindStop(const std::string_view& stopName) const;
        Stop* FindStop(const std::string_view& stopName);
        BusInfo GetBusInfo(std::string_view& busName) const;
        const std::set<std::string>& GetBusesByStop(std::string_view stopName) const;
        void SetDistanceBetweenStops(const Stop* from, const Stop* to, const double distance);
        double GetDistanceBetweenStops(const Stop* from, const Stop* to) const;

    private:
        struct StopPairHash {
            template <typename T, typename U>
            std::size_t operator()(const std::pair<T, U>& p) const {
                auto h1 = std::hash<T>{}(p.first);
                auto h2 = std::hash<U>{}(p.second);
                return h1 ^ h2;
            }
        };

        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Stop*> stopsByName_;
        std::unordered_map<std::string_view, Bus*> busesByName_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopPairHash> distances_;

        double CalculateRouteLength(const Bus& bus) const;
        double CalculateGeoDistance(const Bus& bus) const;
    };

}