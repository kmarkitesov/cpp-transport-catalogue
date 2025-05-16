#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <optional>

namespace transport {

struct RoutingSettings {
    double bus_wait_time = 0;
    double bus_velocity = 0.0;
};

struct RouteItem {
    enum class Type { Wait, Bus };
    Type type;
    std::string name;
    int span_count = 0;
    double time = 0.0;
};

class TransportRouter {
public:
    TransportRouter(const catalogue::TransportCatalogue& db, RoutingSettings settings);

    std::optional<std::vector<RouteItem>> BuildRoute(std::string_view from, std::string_view to) const;

private:
    void BuildGraph();
    double ComputeTravelTime(double distance_meters) const;
    std::unordered_set<const transport::catalogue::Stop*> CollectUniqueStops();
    void InitVerticesAndWaitEdges(const std::unordered_set<const transport::catalogue::Stop*>& unique_stops);
    void AddBusEdges();

    const catalogue::TransportCatalogue& db_;
    RoutingSettings settings_;
    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_;

    std::unordered_map<const catalogue::Stop*, graph::VertexId> stop_to_wait_id_;
    std::unordered_map<const catalogue::Stop*, graph::VertexId> stop_to_board_id_;
    std::vector<RouteItem> edge_items_;
};

} // namespace transport