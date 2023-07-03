#pragma once

#include <unordered_map>
#include <memory>
#include <variant>
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "domain.h"
#include "transport_router.pb.h"

namespace transport_catalogue {

    struct EdgeData {
        size_t id = 0;
        std::string_view name;
        int span = 0;
    };

    struct BusItem {
        std::string_view name;
        double time = 0;
        int span = 0;
    };

    struct WaitItem {
        std::string_view name;
        double time = 0;
    };


    class TransportRouter {
        using Graph = graph::DirectedWeightedGraph<double>;
    public:

        void SetDb(const TransportCatalogue &db);

        void SetBusWaitTime(int time);

        void SetBusVelocity(double velocity);

        void FillGraph();

        std::optional<std::vector<std::variant<BusItem, WaitItem>>>
        GetRoute(std::string_view from, std::string_view to) const;

        void Deserialize(const transport_catalogue_protobuf::TransportRouter &proto_transport_router);

        transport_catalogue_protobuf::TransportRouter Serialize() const;

    private:

        void DeserializeSettings(const transport_catalogue_protobuf::RouteSettings &proto_router_settings);

        transport_catalogue_protobuf::RouteSettings SerializeSettings() const;

        void DeserializeEdgesData(const transport_catalogue_protobuf::EdgesData &proto_edges_data);

        transport_catalogue_protobuf::EdgesData SerializeEdgesData() const;

        template<class StopIter, class DistIter>
        void FillGraphByStopRange(const StopIter begin, const StopIter end, const DistIter dist_vector_begin,
                                  size_t bus_id, std::string_view bus_name);

        void InitializeRouter();

        TransportCatalogue db_;
        Graph graph_;
        std::unique_ptr<graph::Router<double>> router_;

        std::vector<EdgeData> edges_data_;
        int wait_time_ = 1;
        double bus_velocity_ = 1;
    };


}