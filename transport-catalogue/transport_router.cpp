#include "transport_router.h"

namespace transport_catalogue {

    void TransportRouter::SetDb(const TransportCatalogue &db) {
        db_ = db;
    }

    void TransportRouter::SetBusWaitTime(int time) {
        if (time < 1 || time > 1000) {
            throw std::domain_error("Wait time is out of range 1 : 1000");
        }
        wait_time_ = time;
    }

    void TransportRouter::SetBusVelocity(double velocity) {
        if (velocity < 1. || velocity > 1000.) {
            throw std::domain_error("Bus velocity is out of range 1 : 1000");
        }
        bus_velocity_ = velocity * 1'000.0 / 60.0;
    }

    void TransportRouter::FillGraph() {

        graph_ = Graph(db_.GetLastStopId() * 2);
        edges_data_.reserve(db_.GetLastStopId() * 10);

        for (const auto &bus: db_.GetAllBuses()) {

            const auto dist_vec = db_.GetBusRealDistances(bus);

            if (bus->is_circled) {
                FillGraphByStopRange(bus->stops.begin(), bus->stops.end(), dist_vec.begin(), bus->bus_name);
            } else {
                const auto second_end_stop = std::next(bus->stops.begin(), bus->stops.size() / 2);
                FillGraphByStopRange(bus->stops.begin(), std::next(second_end_stop), dist_vec.begin(), bus->bus_name);
                FillGraphByStopRange(second_end_stop, bus->stops.end(),
                                     std::next(dist_vec.begin(), bus->stops.size() / 2), bus->bus_name);
            }
        }
        InitializeRouter();
    }

    void TransportRouter::InitializeRouter() {
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }

    std::optional<std::vector<std::variant<BusItem, WaitItem>>>
    TransportRouter::GetRoute(std::string_view from, std::string_view to) const {
        if (const auto stop_from_info = db_.GetStopInfo(from),
                    stop_to_info = db_.GetStopInfo(to);
                !stop_from_info ||
                !stop_to_info ||
                stop_from_info->buses.empty() ||
                stop_to_info->buses.empty()) {
            return std::nullopt;
        }

        const auto stop_from = db_.FindStop(from);
        const auto stop_to = db_.FindStop(to);
        auto res = router_->BuildRoute(stop_from->id * 2, stop_to->id * 2);

        if (res) {
            std::vector<std::variant<BusItem, WaitItem>> route;
            route.reserve(res->edges.size());
            for (const auto &edge: res->edges) {
                if (edges_data_.at(edge).span) {
                    route.emplace_back(BusItem{edges_data_.at(edge).name,
                                               graph_.GetEdge(edge).weight,
                                               edges_data_.at(edge).span,});
                } else {
                    route.emplace_back(WaitItem{edges_data_.at(edge).name,
                                                graph_.GetEdge(edge).weight});
                }
            }
            return route;
        }
        return std::nullopt;
    }

    template<class StopIter, class DistIter>
    void
    TransportRouter::FillGraphByStopRange(const StopIter begin, const StopIter end,
                                          const DistIter dist_vector_begin,
                                          std::string_view bus_name) {
        int stop_id = 0;

        for (auto stop_it = begin; stop_it != end; ++stop_it, ++stop_id) {
            graph_.AddEdge({(*stop_it)->id * 2,
                            (*stop_it)->id * 2 + 1,
                            1.0 * wait_time_});
            edges_data_.push_back({0, (*stop_it)->stop_name});

            if (stop_it == std::prev(end)) { break; }

            int span = 0;
            int r_length = 0;

            for (auto next_stop = std::next(stop_it); next_stop != end; ++next_stop, ++span) {
                r_length += *std::next(dist_vector_begin, stop_id + span);
                graph_.AddEdge({(*stop_it)->id * 2 + 1,
                                (*next_stop)->id * 2,
                                1.0 * r_length / bus_velocity_});
                edges_data_.push_back({span + 1, bus_name});
            }
        }
    }
}