#include <fstream>
#include <iostream>
#include <string_view>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream &stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char *argv[]) {
    using namespace transport_catalogue;
    using namespace json;


    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    TransportCatalogue tc;
    svg::Document doc;
    MapRenderer mr(doc);
    TransportRouter tr;
    protobuf::SerializableData s_data{&tc, &mr, &tr};
    protobuf::Serializer serializer(s_data);

    JsonRequestProcessor rh(tc, mr, tr, serializer);


    if (mode == "make_base"sv) {

        auto input_json = LoadJSONStream(std::cin);
        PushInputJsonToRH(input_json, rh);
        rh.ParseBaseRequests();
        rh.PushBaseRequest();

//        const auto &bus = tc.FindBus("67"sv);
//
//        int i = 0;
//        int dist = 0;
//        int total_dist = 0;
//        for (const auto &stop: bus->stops) {
//            std::cout << "stop id: "sv << stop->id << " stop name: "sv << stop->stop_name << " dist to next stop: ";
//            dist = tc.GetStopRealDistance(*std::next(bus->stops.begin(), i), *std::next(bus->stops.begin(), i + 1));
//            total_dist += dist;
//            std::cout << dist << " total dist: "<< total_dist << std::endl;
//            ++i;
//        }
//
//        for (const auto& v : tc.GetBusRealDistances(bus)) {
//            std::cout << v << std::endl;
//        }



        serializer.Serialize();
        // make base here

    } else if (mode == "process_requests"sv) {

        auto input_json = LoadJSONStream(std::cin);
        PushInputJsonToRH(input_json, rh);
        serializer.Deserialize();
//        rh.ParseBaseRequests();
//        rh.PushBaseRequest();

        std::cout << rh.PushStatRequests();
    } else if (mode == "simple"sv) {
        auto input_json = LoadJSONStream(std::cin);
        PushInputJsonToRH(input_json, rh);
        rh.ParseBaseRequests();
        rh.PushBaseRequest();
        std::cout << rh.PushStatRequests();
    } else {
        PrintUsage();
        return 1;
    }
}

