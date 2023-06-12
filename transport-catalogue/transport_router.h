#pragma once

#include <unordered_map>
#include <memory>
#include <variant>
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "domain.h"

namespace transport_catalogue {

    struct EdgeData {
        int span = 0;
        std::string_view name;
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

        std::optional<std::vector<std::variant<BusItem, WaitItem>>> GetRoute(std::string_view from, std::string_view to) const;

    private:
        template<class StopIter, class DistIter>
        void FillGraphByStopRange(const StopIter begin, const StopIter end, const DistIter dist_vector_begin,
                                  std::string_view bus_name);

        void InitializeRouter();

        TransportCatalogue db_;
        Graph graph_;
        std::unique_ptr<graph::Router<double>> router_;

        //std::unordered_map<graph::EdgeId, EdgeData> edges_data_
        std::vector<EdgeData> edges_data_;
        int wait_time_ = 1;
        double bus_velocity_ = 1;
    };


}