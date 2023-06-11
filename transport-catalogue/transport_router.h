#pragma once

#include <unordered_map>
#include <memory>
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "domain.h"

namespace transport_catalogue {

    struct EdgeData {
        int span = 0;
        std::string_view name;
    };

    class TransportRouter {
        using Graph = graph::DirectedWeightedGraph<double>;
    public:

        void SetDb(const TransportCatalogue &db);

        void SetBusWaitTime(int time);

        void SetBusVelocity(double velocity);

        void FillGraph();

        void InitializeRouter();

        std::optional<std::vector<std::pair<EdgeData, double>>> GetRoute(std::string_view from, std::string_view to) const;

    private:
        template<class StopIter, class DistIter>
        void FillGraphByStopRange(const StopIter begin, const StopIter end, const DistIter dist_vector_begin,
                                  std::string_view bus_name);


        TransportCatalogue db_;
        Graph graph_;
        std::unique_ptr<graph::Router<double>> router_;

        //std::unordered_map<graph::EdgeId, EdgeData> edges_data_
        std::vector<EdgeData> edges_data_;
        int wait_time_ = 1;
        double bus_velocity_ = 1;
    };


}