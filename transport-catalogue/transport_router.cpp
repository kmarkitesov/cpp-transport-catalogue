#include "transport_router.h"
#include <limits>
#include <unordered_set>

namespace transport {

using namespace catalogue;

constexpr double METERS_IN_KM = 1000.0;
constexpr double MINUTES_IN_HOUR = 60.0;

TransportRouter::TransportRouter(const TransportCatalogue& db, RoutingSettings settings)
    : db_(db), settings_(settings)
{
    BuildGraph();
    router_ = std::make_unique<graph::Router<double>>(graph_);
}

double TransportRouter::ComputeTravelTime(double distance_meters) const {
    return (distance_meters / METERS_IN_KM) / settings_.bus_velocity * MINUTES_IN_HOUR;
}

void TransportRouter::BuildGraph() {
    const auto unique_stops = CollectUniqueStops();
    InitVerticesAndWaitEdges(unique_stops);
    AddBusEdges();
}

std::unordered_set<const Stop*> TransportRouter::CollectUniqueStops() {
    std::unordered_set<const Stop*> result;
    for (const auto& bus : db_.GetBuses()) {
        for (const Stop* stop : bus.stops) {
            result.insert(stop);
        }
    }
    return result;
}

void TransportRouter::InitVerticesAndWaitEdges(const std::unordered_set<const Stop*>& unique_stops) {
    graph_ = graph::DirectedWeightedGraph<double>(unique_stops.size() * 2);
    graph::VertexId vid = 0;

    for (const Stop* stop : unique_stops) {
        const graph::VertexId wait_id = vid++;
        const graph::VertexId board_id = vid++;

        stop_to_wait_id_[stop] = wait_id;
        stop_to_board_id_[stop] = board_id;

        graph::Edge<double> wait_edge{
            wait_id,
            board_id,
            static_cast<double>(settings_.bus_wait_time)
        };
        edge_items_.push_back(RouteItem{RouteItem::Type::Wait, stop->name, 0, settings_.bus_wait_time});
        graph_.AddEdge(wait_edge);
    }
}

void TransportRouter::AddBusEdges() {
    for (const auto& bus : db_.GetBuses()) {
        const auto& stops = bus.stops;
        if (stops.empty()) continue;
        
        for (size_t i = 0; i < stops.size(); ++i) {
            double total_distance = 0.0;
            for (size_t j = i + 1; j < stops.size(); ++j) {
                const Stop* from = stops[i];
                const Stop* to = stops[j];
                if (!from || !to) continue;

                auto from_board_it = stop_to_board_id_.find(from);
                auto to_wait_it = stop_to_wait_id_.find(to);
                if (from_board_it == stop_to_board_id_.end() || to_wait_it == stop_to_wait_id_.end()) continue;

                total_distance += db_.GetDistanceBetweenStops(stops[j - 1], stops[j]);
                double time = ComputeTravelTime(total_distance);
                graph::Edge<double> edge{
                    from_board_it->second,
                    to_wait_it->second,
                    time
                };
                edge_items_.push_back(RouteItem{RouteItem::Type::Bus, bus.name, static_cast<int>(j - i), time});
                graph_.AddEdge(edge);
            }
        }
    }
}

std::optional<std::vector<RouteItem>> TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
    const Stop* from_stop = db_.FindStop(from);
    const Stop* to_stop = db_.FindStop(to);
    if (!from_stop || !to_stop) return std::nullopt;

    auto from_it = stop_to_wait_id_.find(from_stop);
    auto to_it = stop_to_wait_id_.find(to_stop);
    if (from_it == stop_to_wait_id_.end() || to_it == stop_to_wait_id_.end()) return std::nullopt;

    auto from_id = from_it->second;
    auto to_id = to_it->second;
    auto route = router_->BuildRoute(from_id, to_id);
    if (!route) return std::nullopt;

    std::vector<RouteItem> result;
    for (graph::EdgeId edge_id : route->edges) {
        result.push_back(edge_items_.at(edge_id));
    }
    return result;
}

}  // namespace transport