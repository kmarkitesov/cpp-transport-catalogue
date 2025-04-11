#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"
#include <optional>
#include <string_view>
#include <unordered_set>

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

class RequestHandler {
public:
    RequestHandler(const transport::catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer)
        : db_(db), renderer_(renderer) {}

    const transport::catalogue::Bus* FindBus(const std::string_view& bus_name) const { return db_.FindBus(bus_name); }

    const transport::catalogue::Stop* FindStop(std::string_view& name) const { return db_.FindStop(name); }

    transport::catalogue::BusInfo GetBusInfo(std::string_view& name) const { return db_.GetBusInfo(name); }

    svg::Document RenderMap() const { return renderer_.RenderMap(db_); }

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport::catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};

