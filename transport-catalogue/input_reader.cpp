#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <cmath>

namespace transport::input {

    /**
     * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
     */
    geo::Coordinates ParseCoordinates(std::string_view str) {
        static const double nan = std::nan("");

        auto not_space = str.find_first_not_of(' ');
        auto comma = str.find(',');

        if (comma == str.npos) {
            return {nan, nan};
        }

        auto not_space2 = str.find_first_not_of(' ', comma + 1);

        double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
        double lng = std::stod(std::string(str.substr(not_space2)));

        return {lat, lng};
    }

    /**
     * Удаляет пробелы в начале и конце строки
     */
    std::string_view Trim(std::string_view string) {
        const auto start = string.find_first_not_of(' ');
        if (start == string.npos) {
            return {};
        }
        return string.substr(start, string.find_last_not_of(' ') + 1 - start);
    }

    /**
     * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
     */
    std::vector<std::string_view> Split(std::string_view string, char delim) {
        std::vector<std::string_view> result;

        size_t pos = 0;
        while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
            auto delim_pos = string.find(delim, pos);
            if (delim_pos == string.npos) {
                delim_pos = string.size();
            }
            if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                result.push_back(substr);
            }
            pos = delim_pos + 1;
        }

        return result;
    }

    /**
     * Парсит маршрут.
     * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
     * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
     */
    std::vector<std::string_view> ParseRoute(std::string_view route) {
        if (route.find('>') != route.npos) {
            return Split(route, '>');
        }

        auto stops = Split(route, '-');
        std::vector<std::string_view> results(stops.begin(), stops.end());
        results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

        return results;
    }

    std::unordered_map<std::string_view, double> ParseDistances(const std::string_view& description) {
        std::unordered_map<std::string_view, double> distances;
        const std::string_view m_to_str = "m to ";
        auto parts = Split(description, ',');
        for(auto& part : parts){
            part = Trim(part);
            size_t m_pos = part.find(m_to_str);

            if(m_pos != std::string_view::npos){
                size_t start = 0;
                size_t end = m_pos;
                double distance = std::stod(std::string(part.substr(start, end-start)));
                std::string_view neighbor = part.substr(m_pos + m_to_str.size());
                distances[neighbor] = distance;
            }
        }
        return distances;
    }

    CommandDescription ParseCommandDescription(std::string_view line) {
        auto colon_pos = line.find(':');
        if (colon_pos == line.npos) {
            return {};
        }

        auto space_pos = line.find(' ');
        if (space_pos >= colon_pos) {
            return {};
        }

        auto not_space = line.find_first_not_of(' ', space_pos);
        if (not_space >= colon_pos) {
            return {};
        }

        return {std::string(line.substr(0, space_pos)),
                std::string(line.substr(not_space, colon_pos - not_space)),
                std::string(line.substr(colon_pos + 1))};
    }

    void InputReader::ParseLine(std::string_view line) {
        auto command_description = ParseCommandDescription(line);
        if (command_description) {
            commands_.push_back(std::move(command_description));
        }
    }

    void InputReader::ApplyCommands([[maybe_unused]] catalogue::TransportCatalogue& catalogue) const {
        for(const auto& command : commands_){
            if(command.command == "Stop"){
                geo::Coordinates coordinates = ParseCoordinates(command.description);
                catalogue.AddStop(command.id, coordinates);
            }
        }

        for (const auto& command : commands_) {
        if (command.command == "Stop") {
            auto distances = ParseDistances(command.description);
            transport::catalogue::Stop* stop = catalogue.FindStop(command.id);
            for (const auto& [neighbor, distance] : distances) {
                    transport::catalogue::Stop* neighborStop = catalogue.FindStop(neighbor);
                    if (stop && neighborStop) {
                        catalogue.SetDistanceBetweenStops(stop, neighborStop, distance);
                    }
                }
        }
    }

        // for(const auto& command : commands_){
        //     if(command.command == "Bus"){
        //         auto stops = ParseRoute(command.description);
        //         catalogue.AddBus(command.id, stops);
        //     }
        // }
    }

    void ReadBaseRequests(std::istream& input, catalogue::TransportCatalogue& catalogue) {
        int base_request_count;
        input >> base_request_count >> std::ws;

        {
            transport::input::InputReader reader;
            for (int i = 0; i < base_request_count; ++i) {
                std::string line;
                getline(input, line);
                reader.ParseLine(line);
            }
            reader.ApplyCommands(catalogue);
        }
    }
}