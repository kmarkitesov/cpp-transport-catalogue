#include <iostream>
#include <fstream>
#include <sstream>
#include "json.h"
#include "json_reader.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

int main() {
    // Чтение JSON из stdin
    std::string input_json;
    std::getline(std::cin, input_json, '\0');
    std::istringstream input_stream(input_json);
    json::Document input_doc = json::Load(input_stream);
    const json::Dict& root = input_doc.GetRoot().AsMap();;
    // Построение базы данных транспортного справочника
    transport::catalogue::TransportCatalogue catalogue;
    json_reader::JSONReader reader(catalogue);
    const json::Array& base_requests = root.at("base_requests").AsArray();
    const json::Array& stat_requests = root.at("stat_requests").AsArray();
    const json::Dict& render_settings_json = root.at("render_settings").AsMap();
    // Обрабатываем запросы base_requests (остановки и маршруты)
    reader.ProcessBaseRequests(base_requests);
    // Парсинг render_settings из JSON
    renderer::RenderSettings settings = reader.ParseRenderSettings(render_settings_json);
    renderer::MapRenderer renderer(settings);
    // Обработка запросов stat_requests и формирование ответа
    json::Array responses = reader.ProcessStatRequests(stat_requests, renderer);
    json::Document response_doc(responses);
    json::Print(response_doc, std::cout);
    std::cout << std::endl;

    return 0;
}