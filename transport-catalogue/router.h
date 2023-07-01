#pragma once

#include "graph.h"
#include "transport_router.pb.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include <graph.pb.h>
#include <transport_router.pb.h>

namespace graph {

    template<typename Weight>
    class Router {
    private:
        using Graph = DirectedWeightedGraph<Weight>;

    public:


        explicit Router(const Graph &graph);

        explicit Router(const Graph &graph, const transport_catalogue_protobuf::RouterRoutesInternalData &proto_router_routes_internal_data);

        struct RouteInfo {
            Weight weight;
            std::vector<EdgeId> edges;
        };

        std::optional<RouteInfo> BuildRoute(VertexId from, VertexId to) const;

        void Deserialize(const transport_catalogue_protobuf::RouterRoutesInternalData &proto_router_routes_internal_data);

        transport_catalogue_protobuf::RouterRoutesInternalData Serialize() const;


    private:

        struct RouteInternalData {
            Weight weight;
            std::optional<EdgeId> prev_edge;
        };

        using RoutesInternalData = std::vector<std::vector<std::optional<RouteInternalData>>>;

        void InitializeRoutesInternalData(const Graph &graph) {
            const size_t vertex_count = graph.GetVertexCount();
            for (VertexId vertex = 0; vertex < vertex_count; ++vertex) {
                routes_internal_data_[vertex][vertex] = RouteInternalData{ZERO_WEIGHT, std::nullopt};
                for (const EdgeId edge_id: graph.GetIncidentEdges(vertex)) {
                    const auto &edge = graph.GetEdge(edge_id);
                    if (edge.weight < ZERO_WEIGHT) {
                        throw std::domain_error("Edges' weights should be non-negative");
                    }
                    auto &route_internal_data = routes_internal_data_[vertex][edge.to];
                    if (!route_internal_data || route_internal_data->weight > edge.weight) {
                        route_internal_data = RouteInternalData{edge.weight, edge_id};
                    }
                }
            }
        }

        void RelaxRoute(VertexId vertex_from, VertexId vertex_to, const RouteInternalData &route_from,
                        const RouteInternalData &route_to) {
            auto &route_relaxing = routes_internal_data_[vertex_from][vertex_to];
            const Weight candidate_weight = route_from.weight + route_to.weight;
            if (!route_relaxing || candidate_weight < route_relaxing->weight) {
                route_relaxing = {candidate_weight,
                                  route_to.prev_edge ? route_to.prev_edge : route_from.prev_edge};
            }
        }

        void RelaxRoutesInternalDataThroughVertex(size_t vertex_count, VertexId vertex_through) {
            for (VertexId vertex_from = 0; vertex_from < vertex_count; ++vertex_from) {
                if (const auto &route_from = routes_internal_data_[vertex_from][vertex_through]) {
                    for (VertexId vertex_to = 0; vertex_to < vertex_count; ++vertex_to) {
                        if (const auto &route_to = routes_internal_data_[vertex_through][vertex_to]) {
                            RelaxRoute(vertex_from, vertex_to, *route_from, *route_to);
                        }
                    }
                }
            }
        }

        static constexpr Weight ZERO_WEIGHT{};
        const Graph &graph_;
        RoutesInternalData routes_internal_data_;
    };

    template<typename Weight>
    void Router<Weight>::Deserialize(
            const transport_catalogue_protobuf::RouterRoutesInternalData &proto_router_routes_internal_data) {
        routes_internal_data_.clear();
        routes_internal_data_.resize(proto_router_routes_internal_data.routes_internal_data_size());
        auto routes_internal_data_it = routes_internal_data_.begin();

        for (const auto &proto_routes_internal_data: proto_router_routes_internal_data.routes_internal_data()) {
            routes_internal_data_it->resize(proto_routes_internal_data.optional_route_internal_data_vector_size());
            auto optional_route_internal_data_it = routes_internal_data_it->begin();

            for (const auto &proto_optional_route_internal_data: proto_routes_internal_data.optional_route_internal_data_vector()) {
                if (proto_optional_route_internal_data.is_present()) {
                    const auto &proto_route_internal_data = proto_optional_route_internal_data.route_internal_data();
                    auto weight = proto_route_internal_data.weight();

                    *optional_route_internal_data_it = proto_route_internal_data.has_prev_edge() ? std::move(
                            RouteInternalData{weight, proto_route_internal_data.prev_edge()}) : std::move(
                            RouteInternalData{weight, std::nullopt});

                }
                ++optional_route_internal_data_it;
            }
            ++routes_internal_data_it;
        }
    }


    template<typename Weight>
    transport_catalogue_protobuf::RouterRoutesInternalData Router<Weight>::Serialize() const {
        transport_catalogue_protobuf::RouterRoutesInternalData router_routes_internal_data;

        for (const auto &route_internal_data: routes_internal_data_) {
            transport_catalogue_protobuf::RoutesInternalData proto_routes_internal_data;

            for (const auto &internal_data: route_internal_data) {
                transport_catalogue_protobuf::OptionalRouteInternalData proto_optional_route_internal_data;

                if (internal_data) {
                    transport_catalogue_protobuf::RouteInternalData proto_route_internal_data;
                    proto_route_internal_data.set_weight(internal_data->weight);
                    if (internal_data->prev_edge) {
                        proto_route_internal_data.set_prev_edge(internal_data->prev_edge.value());
                        proto_route_internal_data.set_has_prev_edge(true);
                    } else {
                        proto_route_internal_data.set_has_prev_edge(false);
                    }
                    *proto_optional_route_internal_data.mutable_route_internal_data() = std::move(
                            proto_route_internal_data);
                    proto_optional_route_internal_data.set_is_present(true);
                } else {
                    proto_optional_route_internal_data.set_is_present(false);
                }
                *proto_routes_internal_data.add_optional_route_internal_data_vector() = std::move(
                        proto_optional_route_internal_data);
            }
            *router_routes_internal_data.add_routes_internal_data() = std::move(proto_routes_internal_data);
        }
        return router_routes_internal_data;
    }


    template<typename Weight>
    Router<Weight>::Router(const Graph &graph)
            : graph_(graph), routes_internal_data_(graph.GetVertexCount(),
                                                   std::vector<std::optional<RouteInternalData>>(
                                                           graph.GetVertexCount())) {
        InitializeRoutesInternalData(graph);

        const size_t vertex_count = graph.GetVertexCount();
        for (VertexId vertex_through = 0; vertex_through < vertex_count; ++vertex_through) {
            RelaxRoutesInternalDataThroughVertex(vertex_count, vertex_through);
        }
    }

    template<typename Weight>
    Router<Weight>::Router(const Graph &graph, const transport_catalogue_protobuf::RouterRoutesInternalData &proto_router_routes_internal_data) : graph_(graph) {
        Deserialize(proto_router_routes_internal_data);
    }


    template<typename Weight>
    std::optional<typename Router<Weight>::RouteInfo> Router<Weight>::BuildRoute(VertexId from,
                                                                                 VertexId to) const {
        const auto &route_internal_data = routes_internal_data_.at(from).at(to);
        if (!route_internal_data) {
            return std::nullopt;
        }
        const Weight weight = route_internal_data->weight;
        std::vector<EdgeId> edges;
        for (std::optional<EdgeId> edge_id = route_internal_data->prev_edge;
             edge_id;
             edge_id = routes_internal_data_[from][graph_.GetEdge(*edge_id).from]->prev_edge) {
            edges.push_back(*edge_id);
        }
        std::reverse(edges.begin(), edges.end());

        return RouteInfo{weight, std::move(edges)};
    }

}  // namespace graph