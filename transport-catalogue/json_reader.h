#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "request_handler.h"

namespace json_reader {

struct PendingDistance {
    std::string from;
    std::string to;
    int distance;

    PendingDistance(std::string from, std::string to, int distance)
        : from(std::move(from)), to(std::move(to)), distance(distance) {}
};

class JSONReader {
public:
    JSONReader(transport::catalogue::TransportCatalogue& catalogue)
    : catalogue_(catalogue) {};

    void ProcessBaseRequests(const json::Array& base_requests);
    json::Array ProcessStatRequests(const json::Array& stat_requests, const renderer::MapRenderer& renderer);
    renderer::RenderSettings ParseRenderSettings(const json::Dict& dict);

private:
    void ProcessStopRequest(const json::Dict& request);
    void ProcessBusRequest(const json::Dict& request);
    void SetAllPendingDistances();

    json::Dict HandleStopRequest(const json::Dict& request);
    json::Dict HandleBusRequest(const json::Dict& request);
    json::Dict HandleMapRequest(const json::Dict& request, const renderer::MapRenderer& renderer);

    svg::Color ParseColor(const json::Node& node);
    svg::Point ParseOffset(const json::Array& arr);

    transport::catalogue::TransportCatalogue& catalogue_;
    std::vector<PendingDistance> pending_distances_;
};

}