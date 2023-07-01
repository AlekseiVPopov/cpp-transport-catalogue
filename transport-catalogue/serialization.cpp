
#include "serialization.h"

namespace transport_catalogue::protobuf {


//    void Serializer::SerializeStops() {
//        const auto &stops = data_.tc_p->GetStops();
//
//        for (const auto &stop: stops) {
//            transport_catalogue_protobuf::Stop proto_stop;
//
//            proto_stop.set_id(stop.id);
//            proto_stop.set_name(stop.stop_name);
//            proto_stop.set_latitude(stop.coordinates.lat);
//            proto_stop.set_longitude(stop.coordinates.lng);
//            *protobuf_tc_.add_stops() = std::move(proto_stop);
//        }
//    }

//    void Serializer::SerializeBuses() {
//        const auto &buses = data_.tc_p->GetBuses();
//
//        for (const auto &bus: buses) {
//            transport_catalogue_protobuf::Bus proto_bus;
//
//            proto_bus.set_name(bus.bus_name);
//            proto_bus.set_is_circled(bus.is_circled);
//
//            const auto second_bus_end = !bus.is_circled ? std::next(bus.stops.begin(), 1 + (bus.stops.size() / 2))
//                                                        : bus.stops.end();
//
//            for (auto stop_it = bus.stops.begin(); stop_it != second_bus_end; ++stop_it) {
//                proto_bus.add_stops((*stop_it)->id);
//            }
//            *protobuf_tc_.add_buses() = std::move(proto_bus);
//        }
//    }

//    void Serializer::SerializeDistance() {
//        const auto &distances = data_.tc_p->GetNeighbourDistance();
//        for (const auto &[stops_from_to, distance]: distances) {
//            const auto &[from, to] = stops_from_to;
//            transport_catalogue_protobuf::Distance proto_distance;
//
//            proto_distance.set_start_id(from->id);
//            proto_distance.set_end_id(to->id);
//            proto_distance.set_distance(distance);
//            *protobuf_tc_.add_distances() = std::move(proto_distance);
//        }
//
//    }


    bool Serializer::Serialize() {

        if (!data_.tc_p || !data_.mr_p || !data_.tr_p) { return false; }

        protobuf_tc_ = std::move(data_.tc_p->Serialize());
        protobuf_rs_ = std::move(data_.mr_p->Serialize());
        protobuf_tr_ = std::move(data_.tr_p->Serialize());

        return PushToFile();
    }


    bool Serializer::Deserialize() {
        if (!data_.tc_p || !data_.mr_p || !data_.tr_p || !GetFromFile()) { return false; }

        data_.tc_p->Deserialize(protobuf_tc_);
        data_.mr_p->Deserialize(protobuf_rs_);

        data_.tr_p->SetDb(*data_.tc_p);
        data_.tr_p->Deserialize(protobuf_tr_);

        return true;
    }

    void Serializer::SetFilePath(std::string_view file_path) {
        db_file_path_ = std::string(file_path);
    }

    bool Serializer::PushToFile() {
        std::ofstream out(db_file_path_, std::ios::binary);
        if (!out.is_open()) return false;

        transport_catalogue_protobuf::TransportCatalogue proto_catalogue;
        *proto_catalogue.mutable_transport_catalogue() = protobuf_tc_;
        *proto_catalogue.mutable_render_settings() = protobuf_rs_;
        *proto_catalogue.mutable_transport_router() = protobuf_tr_;

        const std::string test = proto_catalogue.SerializeAsString();
        proto_catalogue.SerializeToOstream(&out);
        return true;
    }

    bool Serializer::GetFromFile() {
        std::ifstream in(db_file_path_, std::ios::binary);
        if (!in.is_open()) return false;


        transport_catalogue_protobuf::TransportCatalogue proto_catalogue;
        proto_catalogue.ParseFromIstream(&in);

        protobuf_tc_ = proto_catalogue.transport_catalogue();
        protobuf_rs_ = proto_catalogue.render_settings();
        protobuf_tr_ = proto_catalogue.transport_router();

        return true;
    }

}//end namespace transport_catalogue::protobuf

