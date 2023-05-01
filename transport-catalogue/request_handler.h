#pragma once

#include <unordered_set>

#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"


/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

namespace request_handler {

    using namespace transport_catalogue;
    using namespace json;

    class RequestHandler {
    public:
        // MapRenderer понадобится в следующей части итогового проекта
//        RequestHandler(TransportCatalogue &db, renderer::MapRenderer &renderer) : db_(db), renderer_(
//                renderer) {}
        RequestHandler(TransportCatalogue &db, renderer::MapRenderer &renderer) : db_(db), renderer_(renderer) {
        }


        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<BusInfoResponse> GetBusStat(const std::string_view &bus_name) const;

        // Возвращает маршруты, проходящие через
        std::optional<StopInfoResponse> GetBusesByStop(const std::string_view &stop_name) const;

        void AddRequests(const json::Document &json_doc);

        void ParseBaseRequests();

        void PushBaseRequest();

        void ParseSettings(const json::Node& settings_node);

        std::string GenerateMapToSvg();

        std::string PushStatRequests();

        // Этот метод будет нужен в следующей части итогового проекта
        svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        TransportCatalogue &db_;
        renderer::MapRenderer &renderer_;

        std::deque<const Node *> base_stops_requests_;
        std::deque<const Node *> base_bus_requests_;
        std::deque<const Node *> stat_requests_;
        //Node &settings_requests_;

        std::deque<InputStopInfo> parsed_stop_info_deque_;
        std::deque<InputBusInfo> parsed_bus_info_deque_;
        std::deque<InputDistanceInfo> parsed_distance_info_deque_;
    };
}