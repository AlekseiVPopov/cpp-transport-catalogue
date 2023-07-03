#pragma once

#include <string>
#include <string_view>
#include <algorithm>
#include <deque>
#include <unordered_map>
#include <utility>
#include <stdexcept>
#include <set>
#include <optional>

#include "geo.h"


/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

namespace transport_catalogue {
    struct Stop {
        std::string stop_name;
        geo::Coordinates coordinates;
        size_t id = 0;
    };

    struct Bus {
        std::string bus_name;
        std::vector<Stop *> stops;
        int stops_num = 0;
        int uniq_stops_num = 0;
        bool is_circled = false;
        size_t id = 0;
    };

    struct BusInfoResponse {
        std::string_view bus_name;
        int stops_num = 0;
        int uniq_stops_num = 0;
        double route_length = 0;
        int real_length = 0;
        double curvature = 0;
    };

    struct StopInfoResponse {
        std::string_view stop_name;
        std::vector<Bus *> buses;
    };

    struct InputBusInfo {
        std::string bus_name;
        std::deque<std::string> stops;
        bool is_circled = false;
    };

    struct InputStopInfo {
        std::string stop_name;
        geo::Coordinates coordinates;
    };


    struct InputDistanceInfo {
        std::string stop_name;
        std::unordered_map<std::string, int> distance_to_neighbour;
    };

    namespace detail {

        struct PairOfStopPointersHash {
            std::size_t operator()(const std::pair<const Stop *, const Stop *> &p) const {
                return std::hash<const Stop *>{}(p.first) + 37 * std::hash<const Stop *>{}(p.second);
            }
        };

    }
}