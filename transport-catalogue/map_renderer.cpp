#include <set>
#include "map_renderer.h"
#include "geo.h"
#include "svg.h"

namespace renderer {

using namespace transport::catalogue;

namespace {
constexpr char FONT_VERDANA[] = "Verdana";
constexpr char COLOR_WHITE[] = "white";
constexpr char COLOR_BLACK[] = "black";
constexpr char FONT_WEIGHT_BOLD[] = "bold";
}

MapRenderer::MapRenderer(const RenderSettings& settings)
    : settings_(settings) {}

svg::Document MapRenderer::RenderMap(const TransportCatalogue& db) const {
    svg::Document doc;
    std::vector<geo::Coordinates> all_coords;
    std::vector<const Bus*> buses;

    for (const Bus& bus : db.GetBuses()) {
        if (!bus.stops.empty()) {
            for (const Stop* stop : bus.stops) {
                all_coords.push_back(stop->coords);
            }
            buses.push_back(&bus);
        }
    }

    std::sort(buses.begin(), buses.end(),
        [](const Bus* lhs, const Bus* rhs) {
            return lhs->name < rhs->name;
        });

    SphereProjector projector(all_coords.begin(), all_coords.end(),
                               settings_.width, settings_.height, settings_.padding);

    RenderRoutes(doc, buses, projector);
    RenderBusLabels(doc, buses, projector);

    const auto used_stops = CollectUsedStops(buses);
    RenderStopCircles(doc, used_stops, projector);
    RenderStopLabels(doc, used_stops, projector);

    return doc;
}

std::map<std::string_view, const transport::catalogue::Stop*> MapRenderer::CollectUsedStops(
    const std::vector<const transport::catalogue::Bus*>& buses) const {
    std::set<std::string_view> seen;
    std::map<std::string_view, const Stop*> result;

    for (const auto* bus : buses) {
        for (const auto* stop : bus->stops) {
            if (seen.insert(stop->name).second) {
                result[stop->name] = stop;
            }
        }
    }
    return result;
}

void MapRenderer::RenderRoutes( svg::Document& doc, 
                                const std::vector<const transport::catalogue::Bus*>& buses,
                                const SphereProjector& projector) const {
    size_t color_index = 0;
    for (const Bus* bus : buses) {
        const auto& stops = bus->stops;
        if (stops.empty()) continue;

        svg::Polyline polyline;
        polyline.SetStrokeColor(settings_.color_palette[color_index % settings_.color_palette.size()])
                .SetStrokeWidth(settings_.line_width)
                .SetFillColor(svg::NoneColor)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (const Stop* stop : stops) {
            polyline.AddPoint(projector(stop->coords));
        }

        doc.Add(std::move(polyline));
        ++color_index;
    }
}

void MapRenderer::RenderBusLabels(svg::Document& doc,
                        const std::vector<const transport::catalogue::Bus*>& buses,
                        const SphereProjector& projector) const {
    size_t color_index = 0;
    for (const Bus* bus : buses) {
        if (bus->stops.empty()) continue;

        const svg::Color& color = settings_.color_palette[color_index % settings_.color_palette.size()];
        AddBusLabel(doc, bus->name, projector(bus->stops.front()->coords), color);

        if (!bus->isRoundTrip && bus->originalStopCount > 1) {
            const Stop* last_stop = bus->stops[bus->originalStopCount - 1];
            if (last_stop != bus->stops.front()) {
                svg::Point end_pos = projector(last_stop->coords);
                AddBusLabel(doc, bus->name, end_pos, color);
            }
        }

        ++color_index;
    }
}

void MapRenderer::RenderStopCircles(svg::Document& doc,
                    const std::map<std::string_view, const transport::catalogue::Stop*>& stops,
                    const SphereProjector& projector) const {

    for (const auto& [_, stop] : stops) {
        svg::Circle circle;
        circle.SetCenter(projector(stop->coords))
              .SetRadius(settings_.stop_radius)
              .SetFillColor(COLOR_WHITE);
        doc.Add(std::move(circle));
    }
}

void MapRenderer::RenderStopLabels(svg::Document& doc,
                    const std::map<std::string_view, const transport::catalogue::Stop*>& stops,
                    const SphereProjector& projector) const {
    for (const auto& [_, stop] : stops) {
        svg::Point pos = projector(stop->coords);

        svg::Text underlayer;
        underlayer.SetData(stop->name)
            .SetPosition(pos)
            .SetOffset(settings_.stop_label_offset)
            .SetFontSize(settings_.stop_label_font_size)
            .SetFontFamily(FONT_VERDANA)
            .SetFillColor(settings_.underlayer_color)
            .SetStrokeColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text label;
        label.SetData(stop->name)
            .SetPosition(pos)
            .SetOffset(settings_.stop_label_offset)
            .SetFontSize(settings_.stop_label_font_size)
            .SetFontFamily(FONT_VERDANA)
            .SetFillColor(COLOR_BLACK);

        doc.Add(std::move(underlayer));
        doc.Add(std::move(label));
    }
}

void MapRenderer::AddBusLabel(svg::Document& doc, const std::string& name, svg::Point pos, const svg::Color& color) const {

    svg::Text underlayer;
    underlayer.SetData(name)
        .SetPosition(pos)
        .SetOffset(settings_.bus_label_offset)
        .SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily(FONT_VERDANA)
        .SetFontWeight(FONT_WEIGHT_BOLD)
        .SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    svg::Text label;
        label.SetData(name)
        .SetPosition(pos)
        .SetOffset(settings_.bus_label_offset)
        .SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily(FONT_VERDANA)
        .SetFontWeight(FONT_WEIGHT_BOLD)
        .SetFillColor(color);

    doc.Add(std::move(underlayer));
    doc.Add(std::move(label));
}

}  // namespace renderer
