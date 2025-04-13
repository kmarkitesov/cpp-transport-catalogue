#pragma once

namespace geo {

struct Coordinates {
    double lat; // Широта
    double lng; // Долгота
};

constexpr double EARTH_RADIUS = 6371000.0;

double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo