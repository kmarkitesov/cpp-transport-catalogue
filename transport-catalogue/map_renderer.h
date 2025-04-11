#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include "svg.h"
#include "transport_catalogue.h"
#include "geo.h"

namespace renderer {

struct RenderSettings {
    double width = 0;
    double height = 0;
    double padding = 0;
    double line_width = 0;
    double stop_radius = 0;
    int bus_label_font_size = 0;
    svg::Point bus_label_offset;
    int stop_label_font_size = 0;
    svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width = 0;
    std::vector<svg::Color> color_palette;
};

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (max_lon != min_lon_) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (max_lat_ != min_lat) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer {
public:
    MapRenderer(const RenderSettings& settings);

    svg::Document RenderMap(const transport::catalogue::TransportCatalogue& db) const;

private:
    RenderSettings settings_;

    std::map<std::string_view, const transport::catalogue::Stop*> CollectUsedStops(
        const std::vector<const transport::catalogue::Bus*>& buses) const;
    
    void RenderRoutes(svg::Document& doc, 
                    const std::vector<const transport::catalogue::Bus*>& buses, 
                    const SphereProjector& projector) const;

    void RenderBusLabels(svg::Document& doc,
                        const std::vector<const transport::catalogue::Bus*>& buses,
                        const SphereProjector& projector) const;

    void RenderStopCircles(svg::Document& doc,
                        const std::map<std::string_view, const transport::catalogue::Stop*>& stops,
                        const SphereProjector& projector) const;

    void RenderStopLabels(svg::Document& doc,
                        const std::map<std::string_view, const transport::catalogue::Stop*>& stops,
                        const SphereProjector& projector) const;

    void AddBusLabel(svg::Document& doc, 
                    const std::string& name, 
                    svg::Point pos, 
                    const svg::Color& color) const;
};

}  // namespace renderer