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

#include "domain.h"

namespace transport_catalogue {

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

        int GetStopRealDistance(const Stop *stop1, const Stop *stop2) const;

        std::vector<const Bus *> GetAllBuses();

        //std::vector<const Stop *> GetAllStopWBusses(const std::vector<const Bus *> &busses);
        std::vector<const Stop *> GetAllStopWBusses();


    private:
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Stop *> stop_name_to_stop_;
        std::unordered_map<std::string_view, Bus *> bus_name_to_bus_;
        std::unordered_map<Stop *, std::set<Bus * >> stop_to_buses_;
        std::unordered_map<std::pair<const Stop *, const Stop *>, int, detail::PairOfStopPointersHash> neighbour_distance_;

        static int CountUniqStops(Bus *bus);

    };


}