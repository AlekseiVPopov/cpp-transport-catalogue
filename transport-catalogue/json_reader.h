#pragma once

#include <sstream>

#include "json.h"
#include "json_builder.h"
#include "domain.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "serialization.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace transport_catalogue {

    class JsonRequestProcessor {
    public:
        // MapRenderer понадобится в следующей части итогового проекта

        JsonRequestProcessor(transport_catalogue::TransportCatalogue &db,
                             transport_catalogue::MapRenderer &renderer,
                             transport_catalogue::TransportRouter &router,
                             transport_catalogue::protobuf::Serializer &serializer)
                : db_(db), renderer_(renderer), t_router_(router), serializer_(serializer) {}

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<transport_catalogue::BusInfoResponse> GetBusStat(const std::string_view &bus_name) const;

        // Возвращает маршруты, проходящие через
        std::optional<transport_catalogue::StopInfoResponse> GetBusesByStop(const std::string_view &stop_name) const;

        void AddRequests(const json::Document &json_doc);

        void ParseBaseRequests();

        void PushBaseRequest();

        void ParseRenderSettings(const json::Node &settings_node);

        void ParseRoutingSettings(const json::Node &settings_node);

        void ParseSerializationSettings(const json::Node &settings_node);

        std::string GenerateMapToSvg();

        std::string PushStatRequests();

        // Этот метод будет нужен в следующей части итогового проекта
        svg::Document RenderMap() const;

    private:
        // JsonRequestProcessor использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        transport_catalogue::TransportCatalogue &db_;
        transport_catalogue::MapRenderer &renderer_;
        transport_catalogue::TransportRouter &t_router_;
        transport_catalogue::protobuf::Serializer &serializer_;

        std::deque<const json::Node *> base_stops_requests_;
        std::deque<const json::Node *> base_bus_requests_;
        std::deque<const json::Node *> stat_requests_;
        //Node &settings_requests_;

        std::deque<transport_catalogue::InputStopInfo> parsed_stop_info_deque_;
        std::deque<transport_catalogue::InputBusInfo> parsed_bus_info_deque_;
        std::deque<transport_catalogue::InputDistanceInfo> parsed_distance_info_deque_;
    };


    json::Document LoadJSONString(const std::string &s);

    json::Document LoadJSONStream(std::istream &input);

    void PushInputJsonToRH(json::Document &json_doc, transport_catalogue::JsonRequestProcessor &rh);


    std::string Print(const json::Node &node);


}