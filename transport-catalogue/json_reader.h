#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "request_handler.h"

namespace json_reader {

// Строковые константы для ключей JSON
constexpr char TYPE[] = "type";
constexpr char NAME[] = "name";
constexpr char LATITUDE[] = "latitude";
constexpr char LONGITUDE[] = "longitude";
constexpr char ROAD_DISTANCES[] = "road_distances";
constexpr char STOPS[] = "stops";
constexpr char IS_ROUNDTRIP[] = "is_roundtrip";
constexpr char ID[] = "id";
constexpr char BUSES[] = "buses";
constexpr char ERROR_MESSAGE[] = "error_message";
constexpr char CURVATURE[] = "curvature";
constexpr char ROUTE_LENGTH[] = "route_length";
constexpr char STOP_COUNT[] = "stop_count";
constexpr char UNIQUE_STOP_COUNT[] = "unique_stop_count";
constexpr char REQUEST_ID[] = "request_id";
constexpr char BUS[] = "Bus";
constexpr char STOP[] = "Stop";
constexpr char WIDTH[] = "width";
constexpr char HEIGHT[] = "height";
constexpr char PADDING[] = "padding";
constexpr char LINE_WIDTH[] = "line_width";
constexpr char STOP_RADIUS[] = "stop_radius";
constexpr char BUS_LABEL_FONT_SIZE[] = "bus_label_font_size";
constexpr char BUS_LABEL_OFFSET[] = "bus_label_offset";
constexpr char STOP_LABEL_FONT_SIZE[] = "stop_label_font_size";
constexpr char STOP_LABEL_OFFSET[] = "stop_label_offset";
constexpr char UNDERLAYER_COLOR[] = "underlayer_color";
constexpr char UNDERLAYER_WIDTH[] = "underlayer_width";
constexpr char COLOR_PALETTE[] = "color_palette";
constexpr char MAP[] = "Map";
constexpr char MAP_KEY[] = "map";
const std::string NOT_FOUND = "not found";


void ProcessBaseRequests(const json::Array& base_requests, transport::catalogue::TransportCatalogue& catalogue);

json::Array ProcessStatRequests(const json::Array& stat_requests,
                                const RequestHandler& handler);

renderer::RenderSettings ParseRenderSettings(const json::Dict& dict);

void ProcessStopRequest(const json::Dict& request,
                        transport::catalogue::TransportCatalogue& catalogue,
                        std::vector<std::tuple<std::string, std::string, int>>& pending_distances);

void ProcessBusRequest(const json::Dict& request, transport::catalogue::TransportCatalogue& catalogue);

void SetAllPendingDistances(const std::vector<std::tuple<std::string, std::string, int>>& pending_distances,
                            transport::catalogue::TransportCatalogue& catalogue);

json::Dict HandleStopRequest(const json::Dict& request, const RequestHandler& handler);
json::Dict HandleBusRequest(const json::Dict& request, const RequestHandler& handler);
json::Dict HandleMapRequest(const json::Dict& request, const RequestHandler& handler);

svg::Color ParseColor(const json::Node& node);
svg::Point ParseOffset(const json::Array& arr);

}