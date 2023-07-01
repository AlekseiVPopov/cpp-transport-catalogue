#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"

namespace transport_catalogue {

    void TransportCatalogue::AddStop(const InputStopInfo *stop, const std::optional<size_t> id = std::nullopt) {
        using namespace std::string_literals;
        if (stop == nullptr) {
            throw std::runtime_error("Try to add stop with nullptr"s);
        }

        size_t new_id = id ? *id : last_stop_id_++;

        auto &new_stop = stops_.emplace_back(Stop{stop->stop_name, stop->coordinates, new_id});
        stop_name_to_stop_[new_stop.stop_name] = &new_stop;
    }

    const Stop *TransportCatalogue::FindStop(std::string_view stop_name) const {
        if (stop_name_to_stop_.count(stop_name)) {
            return const_cast<Stop *>(stop_name_to_stop_.at(stop_name));
        }
        return nullptr;
    }

    const Stop *TransportCatalogue::FindStopById(const size_t id) const {
        const auto stop_it = std::next(stops_.begin(), id);

        assert(stop_it != stops_.end() && stop_it->id == id);
        return &(*stop_it);
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
        auto stops_num = bus->is_circled ? bus->stops.size() : bus->stops.size() * 2 - 1;
        stops.reserve(stops_num);

        for (const auto &stop: bus->stops) {
            if (stop_name_to_stop_.find(stop) != stop_name_to_stop_.end()) {
                stops.push_back(stop_name_to_stop_.at(stop));
            } else {
                throw std::runtime_error("No stop "s + stop + " in DB for bus "s + bus->bus_name);
            }
        }

        if (!bus->is_circled) {
            std::copy(next(stops.rbegin()), stops.rend(), std::back_inserter(stops));
        }

        if (*stops.begin() != *stops.rbegin()) {
            throw std::runtime_error("Bus "s + bus->bus_name + " is not closed"s);
        }

        auto &new_bus = buses_.emplace_back(
                std::move(Bus{bus->bus_name,
                              std::move(stops),
                              0,
                              0,
                              bus->is_circled}));
        new_bus.stops_num = static_cast<int>(new_bus.stops.size());
        new_bus.uniq_stops_num = static_cast<int>(CountUniqStops(&new_bus));
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

    int TransportCatalogue::GetStopRealDistance(const Stop *stop1, const Stop *stop2) const {
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
            return {};
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

        for (auto &[stop_name, distance]: distance_info->distance_to_neighbour) {
            auto dest_stop_p = FindStop(stop_name);
            if (!dest_stop_p) {
                throw std::runtime_error(
                        "Stop "s + std::string(stop_name) + " not found to add neighbour distance for "s +
                        std::string(distance_info->stop_name));
            }
            auto p = std::make_pair(base_stop_p, dest_stop_p);
            neighbour_distance_[p] = distance;
        }
    }

    std::vector<const Bus *> TransportCatalogue::GetAllBuses() {
        std::vector<const Bus *> res;
        res.reserve(buses_.size());
        for (const auto &bus: buses_) {
            res.emplace_back(&bus);
        }
        return res;
    }

    //std::vector<const Stop *> TransportCatalogue::GetAllStopWBusses(const std::vector<const Bus *> &busses) {
    std::vector<const Stop *> TransportCatalogue::GetAllStopWBusses() {
        std::vector<const Stop *> res;
        res.reserve(stops_.size());

//        for (const auto &bus: busses) {
//            for (const auto &stop: bus->stops) {
//                res.emplace_back(stop);
//            }
//        }

        for (auto &stop: stops_) {
            if (stop_to_buses_.find(&stop) != stop_to_buses_.end()) {
                res.emplace_back(&stop);

            }
        }

        std::sort(res.begin(), res.end(), [](const auto &lhs, const auto &rhs) {
            return lhs->stop_name < rhs->stop_name;
        });

        return res;
    }

    size_t TransportCatalogue::GetLastStopId() const {
        return last_stop_id_;
    }


    std::vector<int> TransportCatalogue::GetBusRealDistances(const Bus *bus) const {
        std::vector<int> res(bus->stops.size(), -1);

        std::transform(bus->stops.begin(),
                       std::prev(bus->stops.end()),
                       std::next(bus->stops.begin()),
                       res.begin(),
                       [bus, this](const auto &stop1, const auto &stop2) { return GetStopRealDistance(stop1, stop2); });
        res.back() = GetStopRealDistance(*bus->stops.begin(), bus->stops.back());
        return res;
    }

    const std::deque<Stop> &TransportCatalogue::GetStops() const {
        return stops_;
    }

    const std::deque<Bus> &TransportCatalogue::GetBuses() const {
        return buses_;
    }

    std::unordered_map<std::pair<const Stop *, const Stop *>, int, detail::PairOfStopPointersHash> &
    TransportCatalogue::GetNeighbourDistance() {
        return neighbour_distance_;
    }

    void TransportCatalogue::Clear() {
        stops_.clear();
        buses_.clear();
        stop_name_to_stop_.clear();
        bus_name_to_bus_.clear();
        stop_to_buses_.clear();
        neighbour_distance_.clear();
        last_stop_id_ = 0;
    }

    transport_catalogue_protobuf::AllStops TransportCatalogue::SerializeStops() const {
        transport_catalogue_protobuf::AllStops proto_all_stops;

        for (const auto &stop: stops_) {
            transport_catalogue_protobuf::Stop proto_stop;

            proto_stop.set_id(stop.id);
            proto_stop.set_name(stop.stop_name);
            proto_stop.set_latitude(stop.coordinates.lat);
            proto_stop.set_longitude(stop.coordinates.lng);
            *proto_all_stops.add_stops() = std::move(proto_stop);
        }
        return proto_all_stops;
    }

    void TransportCatalogue::DeserializeStops(const transport_catalogue_protobuf::AllStops &proto_all_stops) {
        for (const auto &proto_stop: proto_all_stops.stops()) {
            const InputStopInfo stop_info{proto_stop.name(),
                                          geo::Coordinates{proto_stop.latitude(),
                                                           proto_stop.longitude()}};
            AddStop(&stop_info, proto_stop.id());
        }
        last_stop_id_ = stops_.size();
    }

    transport_catalogue_protobuf::AllBuses TransportCatalogue::SerializeBuses() const {
        transport_catalogue_protobuf::AllBuses proto_all_buses;

        for (const auto &bus: buses_) {
            transport_catalogue_protobuf::Bus proto_bus;

            proto_bus.set_name(bus.bus_name);
            proto_bus.set_is_circled(bus.is_circled);

            const auto second_bus_end = !bus.is_circled ? std::next(bus.stops.begin(), 1 + (bus.stops.size() / 2 ))
                                                        : bus.stops.end();

            for (auto stop_it = bus.stops.begin(); stop_it != second_bus_end; ++stop_it) {
                proto_bus.add_stops((*stop_it)->id);
            }
            *proto_all_buses.add_buses() = std::move(proto_bus);
        }
        return proto_all_buses;
    }

    void TransportCatalogue::DeserializeBuses(const transport_catalogue_protobuf::AllBuses &proto_all_buses) {

        for (const auto &proto_bus: proto_all_buses.buses()) {
            InputBusInfo bus_info;

            bus_info.bus_name = proto_bus.name();
            bus_info.is_circled = proto_bus.is_circled();

            for (const auto &proto_stop_id: proto_bus.stops()) {
                bus_info.stops.emplace_back(FindStopById(proto_stop_id)->stop_name);
            }
            AddBus(&bus_info);
        }
    }

    transport_catalogue_protobuf::TransportCatalogueData TransportCatalogue::Serialize() const {
        transport_catalogue_protobuf::TransportCatalogueData proto_transport_catalogue_data;
        *proto_transport_catalogue_data.mutable_stops() = std::move(SerializeStops());
        *proto_transport_catalogue_data.mutable_buses() = std::move(SerializeBuses());
        *proto_transport_catalogue_data.mutable_distances() = std::move(SerializeDistance());
        return proto_transport_catalogue_data;
    }

    void TransportCatalogue::Deserialize(
            const transport_catalogue_protobuf::TransportCatalogueData &proto_transport_catalogue_data) {
        Clear();
        DeserializeStops(proto_transport_catalogue_data.stops());
        DeserializeBuses(proto_transport_catalogue_data.buses());
        DeserializeDistance(proto_transport_catalogue_data.distances());
    }

    transport_catalogue_protobuf::AllDistances TransportCatalogue::SerializeDistance() const {
        transport_catalogue_protobuf::AllDistances proto_all_distances;

        for (const auto &[stops_from_to, distance]: neighbour_distance_) {
            const auto &[from, to] = stops_from_to;
            transport_catalogue_protobuf::Distance proto_distance;
            proto_distance.set_start_id(from->id);
            proto_distance.set_end_id(to->id);
            proto_distance.set_distance(distance);
            *proto_all_distances.add_distances() = std::move(proto_distance);
        }
        return proto_all_distances;
    }

    void
    TransportCatalogue::DeserializeDistance(const transport_catalogue_protobuf::AllDistances &proto_all_distances) {
        for (const auto &proto_distance: proto_all_distances.distances()) {
            neighbour_distance_.emplace(std::make_pair(FindStopById(proto_distance.start_id()),
                                                       FindStopById(proto_distance.end_id())),
                                        proto_distance.distance());

        }
    }


}