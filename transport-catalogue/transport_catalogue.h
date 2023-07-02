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

#include <transport_catalogue.pb.h>
#include "domain.h"
#include "transport_catalogue.pb.h"

namespace transport_catalogue {

    class TransportCatalogue {
    public:

        void AddStop(const InputStopInfo *stop, const std::optional<size_t> id);

        const Stop *FindStop(std::string_view stop_name) const;

        const Stop *FindStopById(const size_t id) const;

        void AddBus(const InputBusInfo *bus);

        const Bus *FindBus(std::string_view bus_name) const;

        std::optional<BusInfoResponse> GetBusInfo(std::string_view bus_name) const;

        std::optional<StopInfoResponse> GetStopInfo(std::string_view stop_name) const;

        double GetStopDistance(const Stop *stop1, const Stop *stop2) const;

        void AddRealDistance(const InputDistanceInfo *distance_info);

        int GetStopRealDistance(const Stop *stop1, const Stop *stop2) const;

        std::vector<int> GetBusRealDistances(const Bus *bus) const;

        std::vector<const Bus *> GetAllBuses();

        std::vector<const Stop *> GetAllStopWBusses();

        size_t GetLastStopId() const;

        transport_catalogue_protobuf::TransportCatalogueData Serialize() const;

        void Deserialize(const transport_catalogue_protobuf::TransportCatalogueData &proto_transport_catalogue_data);


    private:
        void Clear();

        transport_catalogue_protobuf::AllStops SerializeStops() const;

        void DeserializeStops(const transport_catalogue_protobuf::AllStops &proto_all_stops);

        transport_catalogue_protobuf::AllBuses SerializeBuses() const;

        void DeserializeBuses(const transport_catalogue_protobuf::AllBuses &proto_all_buses);

        transport_catalogue_protobuf::AllDistances SerializeDistance() const;

        void DeserializeDistance(const transport_catalogue_protobuf::AllDistances &proto_all_distances);


        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Stop *> stop_name_to_stop_;
        std::unordered_map<std::string_view, Bus *> bus_name_to_bus_;
        std::unordered_map<Stop *, std::set<Bus * >> stop_to_buses_;
        std::unordered_map<std::pair<const Stop *, const Stop *>, int, detail::PairOfStopPointersHash> neighbour_distance_;

        size_t CountUniqStops(Bus *bus);

        size_t last_stop_id_ = 0;
    };


}