#pragma once

#include <string>
#include <vector>
#include <set>
#include "geo.h"

namespace transport::catalogue {

// Структура, представляющая остановку
struct Stop{
    std::string name;
    geo::Coordinates coords;
    std::set<std::string> buses;
};

// Структура, представляющая автобусный маршрут
struct Bus{
    std::string name;
    std::vector<Stop*> stops;
    bool isRoundTrip = false;
    size_t originalStopCount = 0;
};

// Структура для хранения информации о маршруте
struct BusInfo{
    int stopsCount = 0;
    int uniqueStops = 0;
    double routeLength = 0.0;
    double geoDistance = 0.0;
};

}  // namespace transport::catalogue