#pragma once

#include <string>
#include <string_view>
#include <algorithm>
#include <deque>
#include <unordered_map>
#include <utility>
#include <stdexcept>
#include <set>


#include "geo.h"
#include "input_reader.h"

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
        bool present = false;
    };

    struct StopInfoResponse {
        std::string_view stop_name;
        std::vector<Bus *> buses;
        bool present = false;
    };


    namespace detail {

        struct PairOfStopPointersHash {
            std::size_t operator()(const std::pair<Stop *, Stop *> &p) const {
                return std::hash<Stop *>{}(p.first) + 37 * std::hash<Stop *>{}(p.second);
            }
        };

        struct PtrHasher {
            template<typename T>
            size_t operator()(const T *ptr) const {
                return hasher(ptr);
            }

            std::hash<const void *> hasher;
        };

        template<typename HasherL, typename HasherR>
        struct PairHasher {
            template<typename T, typename S>
            size_t operator()(const std::pair<T, S> &obj) const {
                return hasher_l(obj.first) + 37 * hasher_r(obj.second);
            }

            HasherL hasher_l;
            HasherR hasher_r;
        };
    }

    class TransportCatalogue {


    public:


        void AddStop(InputReader::InputStopInfo *stop);

        Stop *FindStop(std::string_view stop_name) const;

        void AddBus(InputReader::InputBusInfo *bus);

        Bus *FindBus(std::string_view bus_name) const;

        BusInfoResponse GetBusInfo(std::string_view bus_name) const;

        StopInfoResponse GetStopInfo(std::string_view stop_name) const;

        double GetStopDistance(const Stop *stop1, const Stop *stop2) const;

        void AddRealDistance(InputReader::InputDistanceInfo *distance_info);

        size_t GetStopRealDistance(Stop *stop1, Stop *stop2) const;


    private:
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Stop *> stop_name_to_stop_;
        std::unordered_map<std::string_view, Bus *> bus_name_to_bus_;
        std::unordered_map<Stop *, std::set<Bus * >> stop_to_buses_;
        std::unordered_map<std::pair<Stop *, Stop *>, size_t, detail::PairOfStopPointersHash> neighbour_distance_;

        static size_t CountUniqStops(Bus *bus);



    };



}