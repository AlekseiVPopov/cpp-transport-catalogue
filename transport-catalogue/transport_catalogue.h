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

namespace transport_catalogue {

    struct Stop {
        std::string stop_name;
        geo::Coordinates coordinates;
    };


    struct Bus {
        std::string bus_name;
        std::vector<Stop *> stops;
        size_t stops_num = 0;
        size_t uniq_stops_num = 0;
    };

    struct BusInfoResponse {
        std::string_view bus_name;
        size_t stops_num = 0;
        size_t uniq_stops_num = 0;
        double route_length = 0;
        size_t real_length = 0;
        double curvature = 0;
    };

    struct StopInfoResponse {
        std::string_view stop_name;
        std::vector<Bus *> buses;
    };

    struct InputBusInfo {
        std::string bus_name;
        std::deque<std::string_view> stops;
    };

    struct InputStopInfo {
        std::string stop_name;
        geo::Coordinates coordinates;
    };


    struct InputDistanceInfo {
        std::string_view stop_name;
        std::unordered_map<std::string_view, size_t> distance_to_neighbour;
    };


    namespace detail {

        struct PairOfStopPointersHash {
            std::size_t operator()(const std::pair<const Stop *, const Stop *> &p) const {
                return std::hash<const Stop *>{}(p.first) + 37 * std::hash<const Stop *>{}(p.second);
            }
        };

    }

    class TransportCatalogue {


    public:


        void AddStop(const InputStopInfo *stop);

        const Stop *FindStop(std::string_view stop_name) const;

        void AddBus(const InputBusInfo *bus);

        const Bus *FindBus(std::string_view bus_name) const;

        std::optional<BusInfoResponse> GetBusInfo(std::string_view bus_name) const;

        std::optional<StopInfoResponse> GetStopInfo(std::string_view stop_name) const;

        double GetStopDistance(const Stop *stop1, const Stop *stop2) const;

        void AddRealDistance(const InputDistanceInfo *distance_info);

        size_t GetStopRealDistance(const Stop *stop1, const Stop *stop2) const;


    private:
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Stop *> stop_name_to_stop_;
        std::unordered_map<std::string_view, Bus *> bus_name_to_bus_;
        std::unordered_map<Stop *, std::set<Bus * >> stop_to_buses_;
        std::unordered_map<std::pair<const Stop *, const Stop *>, size_t, detail::PairOfStopPointersHash> neighbour_distance_;

        static size_t CountUniqStops(Bus *bus);


    };


}