
#include "serialization.h"

namespace transport_catalogue::protobuf {

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

        //const std::string test = proto_catalogue.SerializeAsString();
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

