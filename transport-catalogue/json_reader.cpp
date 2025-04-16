#include "json_reader.h"
#include <sstream>

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

void JSONReader::ProcessBaseRequests(const json::Array& base_requests) {

    for(const auto& request_node : base_requests) {
        const json::Dict& request = request_node.AsDict();
        std::string type = request.at(json_reader::TYPE).AsString();

        if(type == json_reader::STOP) {
            ProcessStopRequest(request);
        }
    }

    SetAllPendingDistances();
    
    for (const auto& request_node : base_requests) {
        const json::Dict& request = request_node.AsDict();
        std::string type = request.at(json_reader::TYPE).AsString();

        if (type == json_reader::BUS) {
            ProcessBusRequest(request);
        }
    }
}

void JSONReader::ProcessStopRequest(const json::Dict& request) {
    std::string name = request.at(json_reader::NAME).AsString();
    double latitude = request.at(json_reader::LATITUDE).AsDouble();
    double longitude = request.at(json_reader::LONGITUDE).AsDouble();

    geo::Coordinates coords = { latitude, longitude };
    catalogue_.AddStop(name, coords);
    if(request.count(json_reader::ROAD_DISTANCES)) {
        const auto& distances = request.at(json_reader::ROAD_DISTANCES).AsDict();
        for(const auto& [stop_name, distance] : distances) {
            pending_distances_.emplace_back(name, stop_name, distance.AsInt());
        }
    }
}

void JSONReader::ProcessBusRequest(const json::Dict& request) {
    std::string name = request.at(json_reader::NAME).AsString();
    const json::Array& stop_names = request.at(json_reader::STOPS).AsArray();
    std::vector<std::string> stops;

    for (const auto& stop_name : stop_names) {
        stops.push_back(stop_name.AsString());
    }

    bool is_roundtrip = request.at(json_reader::IS_ROUNDTRIP).AsBool();
    catalogue_.AddBus(name, stops, is_roundtrip);
}

void JSONReader::SetAllPendingDistances() {
    for (const auto& [from, to, dist] : pending_distances_) {
        const auto* from_stop = catalogue_.FindStop(from);
        const auto* to_stop = catalogue_.FindStop(to);
        if (from_stop && to_stop) {
            catalogue_.SetDistanceBetweenStops(from_stop, to_stop, dist);
        }
    }
}

json::Array JSONReader::ProcessStatRequests(const json::Array& stat_requests, const renderer::MapRenderer& renderer) {
    json::Array responses;

    for(const auto& stat_request : stat_requests) {
        const json::Dict& request = stat_request.AsDict();
        int request_id = request.at(json_reader::ID).AsInt();
        std::string type = request.at(json_reader::TYPE).AsString();
        json::Dict response;
        response[json_reader::REQUEST_ID] = request_id;

        if(type == json_reader::STOP) {
            responses.emplace_back(HandleStopRequest(request));
        } else if (type == json_reader::BUS) {
            responses.emplace_back(HandleBusRequest(request));
        } else if (type == json_reader::MAP) {
            responses.emplace_back(HandleMapRequest(request, renderer));
        }
    }

    return responses;
}

json::Dict JSONReader::HandleStopRequest(const json::Dict& request) {
    std::string_view stop_name = request.at(json_reader::NAME).AsString();
    const auto* stop = catalogue_.FindStop(stop_name);

    if (!stop) {
        return json::Builder{}.StartDict()
            .Key(REQUEST_ID).Value(request.at(ID).AsInt())
            .Key(ERROR_MESSAGE).Value(NOT_FOUND)
        .EndDict().Build().AsDict();
    } else {
        json::Builder builder;
        auto array_ctx = builder.StartDict()
            .Key(REQUEST_ID).Value(request.at(ID).AsInt())
            .Key(BUSES).StartArray();

        for (const auto& bus : stop->buses) {
            array_ctx.Value(bus);
        }

        return array_ctx.EndArray().EndDict().Build().AsDict();
    }

}

json::Dict JSONReader::HandleBusRequest(const json::Dict& request) {
    std::string_view bus_name = request.at(json_reader::NAME).AsString();
    const transport::catalogue::Bus* bus = catalogue_.FindBus(bus_name);

   if (!bus) {
        return json::Builder{}.StartDict()
            .Key(REQUEST_ID).Value(request.at(ID).AsInt())
            .Key(ERROR_MESSAGE).Value(NOT_FOUND)
        .EndDict().Build().AsDict();
    }

    transport::catalogue::BusInfo bus_info = catalogue_.GetBusInfo(bus_name);
    return json::Builder{}.StartDict()
        .Key(REQUEST_ID).Value(request.at(ID).AsInt())
        .Key(CURVATURE).Value(bus_info.routeLength / bus_info.geoDistance)
        .Key(ROUTE_LENGTH).Value(bus_info.routeLength)
        .Key(STOP_COUNT).Value(bus_info.stopsCount)
        .Key(UNIQUE_STOP_COUNT).Value(bus_info.uniqueStops)
    .EndDict().Build().AsDict();

}

json::Dict JSONReader::HandleMapRequest(const json::Dict& request, const renderer::MapRenderer& renderer) {
    std::ostringstream svg_stream;
    renderer.RenderMap(catalogue_).Render(svg_stream);
    return json::Builder{}.StartDict()
            .Key(REQUEST_ID).Value(request.at(ID).AsInt())
            .Key(MAP_KEY).Value(svg_stream.str())
        .EndDict().Build().AsDict();
}

renderer::RenderSettings JSONReader::ParseRenderSettings(const json::Dict& dict) {
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

svg::Color JSONReader::ParseColor(const json::Node& node) {
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

svg::Point JSONReader::ParseOffset(const json::Array& arr) {
    return { arr[0].AsDouble(), arr[1].AsDouble() };
}

}  // namespace json_reader