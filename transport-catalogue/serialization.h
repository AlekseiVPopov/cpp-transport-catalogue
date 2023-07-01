#pragma once

#include <iostream>
#include <utility>
#include <fstream>
#include <transport_catalogue.pb.h>
#include "transport_catalogue.h"

#include "map_renderer.h"
#include <map_renderer.pb.h>
#include <svg.pb.h>

#include "transport_router.h"
#include <transport_router.pb.h>
#include <graph.pb.h>


namespace transport_catalogue::protobuf {

    struct SerializableData {
        TransportCatalogue *tc_p = nullptr;
        MapRenderer *mr_p = nullptr;
        TransportRouter *tr_p = nullptr;
    };

    class Serializer {
    public:
        Serializer(SerializableData &data) : data_(data) {}

        bool Serialize();

        bool Deserialize();

        void SetFilePath(std::string_view file_path);

    private:

        bool PushToFile();

        bool GetFromFile();


        SerializableData &data_;
        std::string db_file_path_;
        transport_catalogue_protobuf::TransportCatalogueData protobuf_tc_;
        transport_catalogue_protobuf::RenderSettings protobuf_rs_;
        transport_catalogue_protobuf::TransportRouter protobuf_tr_;
    };
}//end namespace Serialization