#pragma once

#include "ranges.h"

#include <cstdlib>
#include <vector>

#include <graph.pb.h>

namespace graph {

    using VertexId = size_t;
    using EdgeId = size_t;


    template<typename Weight>
    struct Edge {
        VertexId from{};
        VertexId to{};
        Weight weight;
    };

    template<typename Weight>
    class DirectedWeightedGraph {
    private:
        using IncidenceList = std::vector<EdgeId>;
        using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

    public:
        DirectedWeightedGraph() = default;

        explicit DirectedWeightedGraph(size_t vertex_count);

        EdgeId AddEdge(const Edge<Weight> &edge);

        size_t GetVertexCount() const;

        size_t GetEdgeCount() const;

        const Edge<Weight> &GetEdge(EdgeId edge_id) const;

        IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

        std::vector<Edge<Weight>> &GetEdges() {
            return edges_;
        }

        std::vector<IncidenceList> &GetIncidenceLists() {
            return incidence_lists_;
        }

        void Deserialize(const transport_catalogue_protobuf::Graph &proto_graph) {

            edges_.clear();
            edges_.reserve(proto_graph.edges_size());
            for (const auto &proto_edge: proto_graph.edges()) {
                edges_.emplace_back(Edge<Weight>{proto_edge.from(), proto_edge.to(), proto_edge.weight()});
            }

            incidence_lists_.clear();
            incidence_lists_.resize(proto_graph.incidence_lists_size());
            auto incidence_lists_it = incidence_lists_.begin();

            for (const auto &proto_incident_list: proto_graph.incidence_lists()) {
                *incidence_lists_it = std::vector<graph::EdgeId>(proto_incident_list.values_size());
                for (const auto &value: proto_incident_list.values()) {
                    incidence_lists_it->emplace_back(value);
                }
                ++incidence_lists_it;
            }
        }

        transport_catalogue_protobuf::Graph Serialize() const {
            transport_catalogue_protobuf::Graph proto_graph;
            for (const auto &edge: edges_) {
                transport_catalogue_protobuf::Edge proto_edge;
                proto_edge.set_from(edge.from);
                proto_edge.set_to(edge.to);
                proto_edge.set_weight(edge.weight);
                *proto_graph.add_edges() = std::move(proto_edge);
            }

            for (const auto &incidence_list: incidence_lists_) {
                transport_catalogue_protobuf::IncidenceList proto_incidence_list;
                for (const auto &incidence: incidence_list) {
                    proto_incidence_list.add_values(incidence);
                }
                *proto_graph.add_incidence_lists() = std::move(proto_incidence_list);
            }

            return proto_graph;
        }

    private:
        std::vector<Edge<Weight>> edges_;
        std::vector<IncidenceList> incidence_lists_;
    };

    template<typename Weight>
    DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
            : incidence_lists_(vertex_count) {
    }

    template<typename Weight>
    EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight> &edge) {
        edges_.push_back(edge);
        const EdgeId id = edges_.size() - 1;
        incidence_lists_.at(edge.from).push_back(id);
        return id;
    }

    template<typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
        return incidence_lists_.size();
    }

    template<typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
        return edges_.size();
    }

    template<typename Weight>
    const Edge<Weight> &DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
        return edges_.at(edge_id);
    }

    template<typename Weight>
    typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
    DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
        return ranges::AsRange(incidence_lists_.at(vertex));
    }
}  // namespace graph