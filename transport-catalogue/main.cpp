#include <iostream>
#include <fstream>
#include <sstream>
#include "json.h"
#include "json_reader.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace json_fields {
    inline constexpr std::string_view BASE_REQUESTS = "base_requests";
    inline constexpr std::string_view STAT_REQUESTS = "stat_requests";
    inline constexpr std::string_view RENDER_SETTINGS = "render_settings";
    inline constexpr std::string_view ROUTING_SETTINGS = "routing_settings";
}

int main() {
    // Чтение JSON из stdin
    std::string input_json;
    std::getline(std::cin, input_json, '\0');
    std::istringstream input_stream(input_json);
    json::Document input_doc = json::Load(input_stream);
    const json::Dict& root = input_doc.GetRoot().AsDict();;
    // Построение базы данных транспортного справочника
    transport::catalogue::TransportCatalogue catalogue;
    json_reader::JSONReader reader(catalogue);
    const json::Array& base_requests = root.at(std::string(json_fields::BASE_REQUESTS)).AsArray();
    const json::Array& stat_requests = root.at(std::string(json_fields::STAT_REQUESTS)).AsArray();
    const json::Dict& render_settings_json = root.at(std::string(json_fields::RENDER_SETTINGS)).AsDict();
    const json::Dict& routing_settings_json = root.at(std::string(json_fields::ROUTING_SETTINGS)).AsDict();
    // Обрабатываем запросы base_requests (остановки и маршруты)
    reader.ProcessBaseRequests(base_requests);
    // Парсинг render_settings из JSON
    renderer::RenderSettings settings = reader.ParseRenderSettings(render_settings_json);
    renderer::MapRenderer renderer(settings);
    // Настройки маршрутизатора
    transport::RoutingSettings routing_settings = reader.ParseRoutingSettings(routing_settings_json);
    reader.SetRouter(routing_settings);
    // Обработка запросов stat_requests и формирование ответа
    json::Array responses = reader.ProcessStatRequests(stat_requests, renderer);
    json::Document response_doc(responses);
    json::Print(response_doc, std::cout);
    std::cout << std::endl;

    return 0;
}