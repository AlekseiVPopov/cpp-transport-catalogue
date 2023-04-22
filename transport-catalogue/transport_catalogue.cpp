#include "transport_catalogue.h"

namespace transport_catalogue {

    void TransportCatalogue::AddStop(const InputStopInfo *stop) {
        using namespace std::string_literals;
        if (stop == nullptr) {
            throw std::runtime_error("Try to add stop with nullptr"s);
        }
        auto &new_stop = stops_.emplace_back(Stop{stop->stop_name, stop->coordinates});
        stop_name_to_stop_[new_stop.stop_name] = &new_stop;
    }


    const Stop *TransportCatalogue::FindStop(std::string_view stop_name) const {
        if (stop_name_to_stop_.count(stop_name)) {
            return const_cast<Stop *>(stop_name_to_stop_.at(stop_name));
        }
        return nullptr;
    }

    size_t TransportCatalogue::CountUniqStops(Bus *bus) {
        std::set<std::string_view> uniq_stop_names;
        for (auto stop: bus->stops) {
            uniq_stop_names.insert(stop->stop_name);
        }
        return uniq_stop_names.size();
    }

    void TransportCatalogue::AddBus(const InputBusInfo *bus) {
        using namespace std::string_literals;

        if (bus == nullptr) {
            throw std::runtime_error("Try to add bus with nullptr"s);
        }
        std::vector<Stop *> stops;

        for (auto stop: bus->stops) {
            if (stop_name_to_stop_.find(stop) != stop_name_to_stop_.end()) {
                stops.push_back(stop_name_to_stop_.at(stop));
            } else {
                throw std::runtime_error("No stop "s + std::string(stop) + " in DB for bus "s + bus->bus_name);
            }
        }
        auto &new_bus = buses_.emplace_back(Bus{bus->bus_name, std::move(stops)});
        new_bus.stops_num = new_bus.stops.size();
        new_bus.uniq_stops_num = CountUniqStops(&new_bus);
        bus_name_to_bus_[new_bus.bus_name] = &new_bus;

        for (auto stop: new_bus.stops) {
            stop_to_buses_[stop].insert(&new_bus);
        }


    }


    const Bus *TransportCatalogue::FindBus(std::string_view bus_name) const {
        if (bus_name_to_bus_.count(bus_name)) {
            return const_cast<Bus *>(bus_name_to_bus_.at(bus_name));
        }
        return nullptr;
    }

    double TransportCatalogue::GetStopDistance(const Stop *stop1, const Stop *stop2) const {
        return geo::ComputeDistance(stop1->coordinates, stop2->coordinates);
    }

    size_t TransportCatalogue::GetStopRealDistance(const Stop *stop1, const Stop *stop2) const {
        if (auto direct = neighbour_distance_.find(std::make_pair(stop1, stop2)); direct == neighbour_distance_.end()) {
            if (auto reverse = neighbour_distance_.find(std::make_pair(stop2, stop1)); reverse ==
                                                                                       neighbour_distance_.end()) {
                return 0;
            } else {
                return reverse->second;
            }
        } else {
            return direct->second;
        }
    }


    std::optional<BusInfoResponse> TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
        BusInfoResponse res;
        res.bus_name = bus_name;


        if (bus_name_to_bus_.find(bus_name) == bus_name_to_bus_.end()) {
            return res;
        }
        auto bus = bus_name_to_bus_.at(bus_name);

        for (auto begin_it = bus->stops.begin();
             begin_it != bus->stops.end(); ++begin_it) {
            if (next(begin_it) == bus->stops.end()) {
                break;
            }
            res.route_length += GetStopDistance(*begin_it, *next(begin_it));
            res.real_length += GetStopRealDistance(*begin_it, *next(begin_it));
        }

        res.curvature = 1.0 * res.real_length / res.route_length;

        res.stops_num = bus->stops_num;
        res.uniq_stops_num = bus->uniq_stops_num;
        return res;
    }

    std::optional<StopInfoResponse> TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
        auto stop_it = stop_name_to_stop_.find(stop_name);
        if (stop_it == stop_name_to_stop_.end()) {
            return {};
        }

        if (stop_to_buses_.find(stop_it->second) == stop_to_buses_.end()) {
            return StopInfoResponse{stop_name, {}};
        }

        auto buses = stop_to_buses_.at(stop_it->second);

        std::vector<Bus *> bus_p_vec{buses.begin(), buses.end()};
        std::sort(bus_p_vec.begin(), bus_p_vec.end(), [](auto lhs, auto rhs) {
            return lhs->bus_name < rhs->bus_name;
        });

        return StopInfoResponse{stop_name, bus_p_vec};
    }

    void TransportCatalogue::AddRealDistance(const InputDistanceInfo *distance_info) {
        using namespace std::string_literals;
        if (distance_info == nullptr) {
            throw std::runtime_error("Try to add stop with nullptr"s);
        }

        auto base_stop_p = FindStop(distance_info->stop_name);
        if (!base_stop_p) {
            throw std::runtime_error("Stop "s + std::string(distance_info->stop_name) + " not found to add distance"s);
        }

        for (auto [stop_name, distance]: distance_info->distance_to_neighbour) {
            auto dest_stop_p = FindStop(stop_name);
            if (!dest_stop_p) {
                throw std::runtime_error(
                        "Stop "s + std::string(distance_info->stop_name) + " not found to add neighbour distance"s);
            }
            auto p = std::make_pair(base_stop_p, dest_stop_p);
            neighbour_distance_[p] = distance;
        }
    }
}