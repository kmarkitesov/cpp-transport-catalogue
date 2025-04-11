#include "json_reader.h"
#include <sstream>

namespace json_reader {

void ProcessBaseRequests(const json::Array& base_requests, transport::catalogue::TransportCatalogue& catalogue) {
    std::vector<std::tuple<std::string, std::string, int>> pending_distances;

    for(const auto& request_node : base_requests) {
        const json::Dict& request = request_node.AsMap();
        std::string type = request.at(json_reader::TYPE).AsString();

        if(type == json_reader::STOP) {
            ProcessStopRequest(request, catalogue, pending_distances);
        }
    }

    SetAllPendingDistances(pending_distances, catalogue);
    
    for (const auto& request_node : base_requests) {
        const json::Dict& request = request_node.AsMap();
        std::string type = request.at(json_reader::TYPE).AsString();

        if (type == json_reader::BUS) {
            ProcessBusRequest(request, catalogue);
        }
    }
}

void ProcessStopRequest(const json::Dict& request,
                        transport::catalogue::TransportCatalogue& catalogue,
                        std::vector<std::tuple<std::string, std::string, int>>& pending_distances) {
    std::string name = request.at(json_reader::NAME).AsString();
    double latitude = request.at(json_reader::LATITUDE).AsDouble();
    double longitude = request.at(json_reader::LONGITUDE).AsDouble();

    geo::Coordinates coords = { latitude, longitude };
    catalogue.AddStop(name, coords);
    if(request.count(json_reader::ROAD_DISTANCES)) {
        const auto& distances = request.at(json_reader::ROAD_DISTANCES).AsMap();
        for(const auto& [stop_name, distance] : distances) {
            pending_distances.emplace_back(name, stop_name, distance.AsInt());
        }
    }
}

void ProcessBusRequest(const json::Dict& request, transport::catalogue::TransportCatalogue& catalogue) {
    std::string name = request.at(json_reader::NAME).AsString();
    const json::Array& stop_names = request.at(json_reader::STOPS).AsArray();
    std::vector<std::string> stops;

    for (const auto& stop_name : stop_names) {
        stops.push_back(stop_name.AsString());
    }

    bool is_roundtrip = request.at(json_reader::IS_ROUNDTRIP).AsBool();
    catalogue.AddBus(name, stops, is_roundtrip);
}

void SetAllPendingDistances(const std::vector<std::tuple<std::string, std::string, int>>& pending_distances,
                            transport::catalogue::TransportCatalogue& catalogue) {
    for (const auto& [from, to, dist] : pending_distances) {
        const auto* from_stop = catalogue.FindStop(from);
        const auto* to_stop = catalogue.FindStop(to);
        if (from_stop && to_stop) {
            catalogue.SetDistanceBetweenStops(from_stop, to_stop, dist);
        }
    }
}

json::Array ProcessStatRequests(const json::Array& stat_requests,
                                const RequestHandler& handler) {
    json::Array responses;

    for(const auto& stat_request : stat_requests) {
        const json::Dict& request = stat_request.AsMap();
        int request_id = request.at(json_reader::ID).AsInt();
        std::string type = request.at(json_reader::TYPE).AsString();
        json::Dict response;
        response[json_reader::REQUEST_ID] = request_id;

        if(type == json_reader::STOP) {
            responses.emplace_back(HandleStopRequest(request, handler));
        } else if (type == json_reader::BUS) {
            responses.emplace_back(HandleBusRequest(request, handler));
        } else if (type == json_reader::MAP) {
            responses.emplace_back(HandleMapRequest(request, handler));
        }
    }

    return responses;
}

json::Dict HandleStopRequest(const json::Dict& request, const RequestHandler& handler) {
    json::Dict response;
    response[json_reader::REQUEST_ID] = request.at(json_reader::ID).AsInt();
    std::string_view stop_name = request.at(json_reader::NAME).AsString();
    const auto* stop = handler.FindStop(stop_name);

    if (!stop) {
        response[json_reader::ERROR_MESSAGE] = json_reader::NOT_FOUND;
    } else {
        json::Array buses_array;
        for (const auto& bus : stop->buses) {
            buses_array.push_back(bus);
        }
        response[json_reader::BUSES] = buses_array;
    }

    return response;
}

json::Dict HandleBusRequest(const json::Dict& request, const RequestHandler& handler) {
    json::Dict response;
    response[json_reader::REQUEST_ID] = request.at(json_reader::ID).AsInt();
    std::string_view bus_name = request.at(json_reader::NAME).AsString();
    const transport::catalogue::Bus* bus = handler.FindBus(bus_name);

    if(!bus) {
        response[json_reader::ERROR_MESSAGE] = json_reader::NOT_FOUND;
    } else {
        transport::catalogue::BusInfo bus_info = handler.GetBusInfo(bus_name);
        response[json_reader::CURVATURE] = bus_info.routeLength / bus_info.geoDistance;
        response[json_reader::ROUTE_LENGTH] = bus_info.routeLength;
        response[json_reader::STOP_COUNT] = bus_info.stopsCount;
        response[json_reader::UNIQUE_STOP_COUNT] = bus_info.uniqueStops;
    }

    return response;
}

json::Dict HandleMapRequest(const json::Dict& request, const RequestHandler& handler) {
    json::Dict response;
    response[json_reader::REQUEST_ID] = request.at(json_reader::ID).AsInt();
    std::ostringstream svg_stream;
    handler.RenderMap().Render(svg_stream);
    response[json_reader::MAP_KEY] = svg_stream.str();
    return response;
}

renderer::RenderSettings ParseRenderSettings(const json::Dict& dict) {
    renderer::RenderSettings settings;
    settings.width = dict.at(json_reader::WIDTH).AsDouble();
    settings.height = dict.at(json_reader::HEIGHT).AsDouble();
    settings.padding = dict.at(json_reader::PADDING).AsDouble();
    settings.line_width = dict.at(json_reader::LINE_WIDTH).AsDouble();
    settings.stop_radius = dict.at(json_reader::STOP_RADIUS).AsDouble();
    settings.bus_label_font_size = dict.at(json_reader::BUS_LABEL_FONT_SIZE).AsInt();
    settings.bus_label_offset = ParseOffset(dict.at(json_reader::BUS_LABEL_OFFSET).AsArray());
    settings.stop_label_font_size = dict.at(json_reader::STOP_LABEL_FONT_SIZE).AsInt();
    settings.stop_label_offset = ParseOffset(dict.at(json_reader::STOP_LABEL_OFFSET).AsArray());
    settings.underlayer_color = ParseColor(dict.at(json_reader::UNDERLAYER_COLOR));
    settings.underlayer_width = dict.at(json_reader::UNDERLAYER_WIDTH).AsDouble();
    const auto& palette = dict.at(json_reader::COLOR_PALETTE).AsArray();
    for (const auto& color : palette) {
        settings.color_palette.push_back(ParseColor(color));
    }

    return settings;
}

svg::Color ParseColor(const json::Node& node) {
    if (node.IsString()) {
        return node.AsString();
    } else if (node.IsArray()) {
        const auto& arr = node.AsArray();
        if (arr.size() == 3) {
            return svg::Rgb{
                static_cast<uint8_t>(arr[0].AsInt()),
                static_cast<uint8_t>(arr[1].AsInt()),
                static_cast<uint8_t>(arr[2].AsInt())
            };
        } else if (arr.size() == 4) {
            return svg::Rgba{
                static_cast<uint8_t>(arr[0].AsInt()),
                static_cast<uint8_t>(arr[1].AsInt()),
                static_cast<uint8_t>(arr[2].AsInt()),
                arr[3].AsDouble()
            };
        }
    }
    
    return svg::NoneColor;
}

svg::Point ParseOffset(const json::Array& arr) {
    return { arr[0].AsDouble(), arr[1].AsDouble() };
}

}  // namespace json_reader