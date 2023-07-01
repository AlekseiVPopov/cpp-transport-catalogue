#include "json_reader.h"
#include <unordered_set>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace transport_catalogue {

    using namespace transport_catalogue;
    using namespace json;
    using namespace std::string_literals;

    std::optional<BusInfoResponse> JsonRequestProcessor::GetBusStat(const std::string_view &bus_name) const {
        return db_.GetBusInfo(bus_name);
    }

    std::optional<StopInfoResponse> JsonRequestProcessor::GetBusesByStop(const std::string_view &stop_name) const {
        return db_.GetStopInfo(stop_name);
    }

    void JsonRequestProcessor::AddRequests(const json::Document &json_doc) {
        try {
            const auto &doc = json_doc.GetRoot();

            for (const auto &[category, cat_value]: doc.AsDict()) {
                if (category == "base_requests"s) {
                    for (const auto &req_value: cat_value.AsArray()) {
                        const auto &command = req_value.AsDict();
                        if (command.at("type"s).AsString() == "Stop"s) {
                            base_stops_requests_.push_back(&req_value);
                        } else if (command.at("type"s).AsString() == "Bus"s) {
                            base_bus_requests_.push_back(&req_value);
                        }
                    }
                } else if (category == "stat_requests"s) {
                    for (const auto &req_value: cat_value.AsArray()) {
                        stat_requests_.push_back(&req_value);
                    }
                } else if (category == "render_settings"s && !cat_value.AsDict().empty()) {
                    ParseRenderSettings(cat_value);
                } else if (category == "routing_settings"s && !cat_value.AsDict().empty()) {
                    ParseRoutingSettings(cat_value);
                } else if (category == "serialization_settings"s && !cat_value.AsDict().empty()) {
                    ParseSerializationSettings(cat_value);
                }
            }
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    }


    void JsonRequestProcessor::ParseBaseRequests() {
        for (const auto stop_jnode: base_stops_requests_) {
            const auto &stop_map = stop_jnode->AsDict();
            InputStopInfo stop_info{stop_map.at("name"s).AsString(),
                                    {stop_map.at("latitude"s).AsDouble(),
                                     stop_map.at("longitude"s).AsDouble()}};
            parsed_stop_info_deque_.emplace_back(std::move(stop_info));

            const auto &dist_info_map = stop_map.at("road_distances"s).AsDict();
            if (!dist_info_map.empty()) {
                InputDistanceInfo dist_info{stop_map.at("name"s).AsString(), {}};
                for (const auto &[neigh_name, dist]: dist_info_map) {
                    dist_info.distance_to_neighbour.insert(std::make_pair(neigh_name, dist.AsInt()));
                }
                parsed_distance_info_deque_.emplace_back(std::move(dist_info));
            }
        }


        for (const auto bus_jnode: base_bus_requests_) {
            const auto &bus_map = bus_jnode->AsDict();
            InputBusInfo bus_info{bus_map.at("name"s).AsString(), {}};

            std::vector<std::string> stops_vec;

            for (const auto &stops: bus_map.at("stops").AsArray()) {
                stops_vec.emplace_back(stops.AsString());
            }
            std::deque<std::string> stops_v_deque{stops_vec.begin(), stops_vec.end()};
            bus_info.is_circled = bus_map.at("is_roundtrip"s).AsBool();
            bus_info.stops = std::move(stops_v_deque);
            parsed_bus_info_deque_.emplace_back(std::move(bus_info));
        }

        if (!parsed_bus_info_deque_.empty()) {
            std::sort(parsed_bus_info_deque_.begin(), parsed_bus_info_deque_.end(),
                      [](const auto &lhs, const auto &rhs) {
                          return lhs.bus_name < rhs.bus_name;
                      });
        }

        if (!parsed_stop_info_deque_.empty()) {
            std::sort(parsed_stop_info_deque_.begin(), parsed_stop_info_deque_.end(),
                      [](const auto &lhs, const auto &rhs) {
                          return lhs.stop_name < rhs.stop_name;
                      });
        }

    }


    void JsonRequestProcessor::ParseRenderSettings(const Node &settings_node) {

        transport_catalogue::RenderSettings rs;

        for (const auto &[key, value]: settings_node.AsDict()) {
            if (key == "width"s) {
                rs.width = value.AsDouble();
            }
            if (key == "height"s) {
                rs.height = value.AsDouble();
            }
            if (key == "padding"s) {
                rs.padding = value.AsDouble();
            }
            if (key == "stop_radius"s) {
                rs.stop_radius = value.AsDouble();
            }
            if (key == "line_width"s) {
                rs.line_width = value.AsDouble();
            }
            if (key == "bus_label_font_size"s) {
                rs.bus_label_font_size = value.AsInt();
            }
            if (key == "bus_label_offset"s) {
                const auto &offset_arr_node = value.AsArray();
                rs.bus_label_offset = {offset_arr_node.at(0).AsDouble(), offset_arr_node.at(1).AsDouble()};

            }
            if (key == "stop_label_font_size"s) {
                rs.stop_label_font_size = value.AsInt();
            }
            if (key == "stop_label_offset"s) {
                const auto &offset_arr_node = value.AsArray();
                rs.stop_label_offset = {offset_arr_node.at(0).AsDouble(), offset_arr_node.at(1).AsDouble()};
            }
            if (key == "underlayer_color"s) {
                if (value.IsString()) {
                    rs.underlayer_color = svg::Color(value.AsString());
                }
                if (value.IsArray()) {
                    const auto &color_arr = value.AsArray();
                    if (color_arr.size() == 3) {
                        rs.underlayer_color = svg::Color(
                                svg::Rgb(color_arr[0].AsInt(),
                                         color_arr[1].AsInt(),
                                         color_arr[2].AsInt()));
                    } else if (color_arr.size() == 4) {
                        rs.underlayer_color = svg::Color(
                                svg::Rgba(color_arr[0].AsInt(),
                                          color_arr[1].AsInt(),
                                          color_arr[2].AsInt(),
                                          color_arr[3].AsDouble()));
                    }
                }
            }
            if (key == "underlayer_width"s) {
                rs.underlayer_width = value.AsDouble();
            }
            if (key == "color_palette"s) {

                rs.color_palette.clear();

                for (const auto &palette_arr_node: value.AsArray()) {
                    if (palette_arr_node.IsString()) {
                        rs.color_palette.emplace_back(palette_arr_node.AsString());
                    }
                    if (palette_arr_node.IsArray()) {
                        const auto &color_arr = palette_arr_node.AsArray();
                        if (color_arr.size() == 3) {
                            rs.color_palette.emplace_back(svg::Rgb(color_arr[0].AsInt(),
                                                                   color_arr[1].AsInt(),
                                                                   color_arr[2].AsInt()));
                        } else if (color_arr.size() == 4) {
                            rs.color_palette.emplace_back(svg::Rgba(color_arr[0].AsInt(),
                                                                    color_arr[1].AsInt(),
                                                                    color_arr[2].AsInt(),
                                                                    color_arr[3].AsDouble()));
                        }
                    }
                }
            }
        }
        renderer_.SetSettings(std::move(rs));
    }

    void JsonRequestProcessor::ParseRoutingSettings(const Node &settings_node) {

        for (const auto &[key, value]: settings_node.AsDict()) {
            if (key == "bus_wait_time"s) {
                t_router_.SetBusWaitTime(value.AsInt());
            }
            if (key == "bus_velocity"s) {
                t_router_.SetBusVelocity(value.AsDouble());
            }
        }
    }

    void JsonRequestProcessor::ParseSerializationSettings(const json::Node &settings_node) {
        for (const auto &[key, value]: settings_node.AsDict()) {
            if (key == "file"s) {
                serializer_.SetFilePath(value.AsString());
            }
        }
    }

    void JsonRequestProcessor::PushBaseRequest() {

        for (const auto &stop: parsed_stop_info_deque_) {
            db_.AddStop(&stop, std::nullopt);
        }

        for (const auto &bus: parsed_bus_info_deque_) {
            db_.AddBus(&bus);
        }

        for (const auto &dist: parsed_distance_info_deque_) {
            db_.AddRealDistance(&dist);
        }

        t_router_.SetDb(db_);
        t_router_.FillGraph();
    }

    std::string JsonRequestProcessor::PushStatRequests() {

        Array arr;
        arr.reserve(1'000);

        for (const auto stat_jnode: stat_requests_) {
            const auto &stat_map = stat_jnode->AsDict();
            if (stat_map.at("type"s).AsString() == "Bus"s) {
                const auto res = db_.GetBusInfo(stat_map.at("name"s).AsString());
                if (res) {
                    const auto &res_val = res.value();
                    arr.emplace_back(json::Builder{}.StartDict()
                                             .Key("request_id"s).Value(stat_map.at("id").AsInt())
                                             .Key("curvature"s).Value(res_val.curvature)
                                             .Key("route_length"s).Value(res_val.real_length)
                                             .Key("stop_count"s).Value(res_val.stops_num)
                                             .Key("unique_stop_count"s).Value(res_val.uniq_stops_num)
                                             .EndDict().Build().GetRoot());

                } else {
                    arr.emplace_back(json::Builder{}.StartDict()
                                             .Key("request_id"s).Value(stat_map.at("id").AsInt())
                                             .Key("error_message"s).Value("not found"s)
                                             .EndDict().Build().GetRoot());

                }
            } else if (stat_map.at("type"s).AsString() == "Stop"s) {
                auto res = db_.GetStopInfo(stat_map.at("name"s).AsString());

                if (res) {
                    const auto &res_val = res.value();
                    Array bus_arr;
                    for (const auto &bus: res_val.buses) {
                        bus_arr.emplace_back(bus->bus_name);
                    }
                    arr.emplace_back(json::Builder{}.StartDict()
                                             .Key("request_id"s).Value(stat_map.at("id").AsInt())
                                             .Key("buses").Value(bus_arr)
                                             .EndDict().Build().GetRoot());

                } else {
                    arr.emplace_back(json::Builder{}.StartDict()
                                             .Key("request_id"s).Value(stat_map.at("id").AsInt())
                                             .Key("error_message"s).Value("not found"s)
                                             .EndDict().Build().GetRoot());
                }
            } else if (stat_map.at("type"s).AsString() == "Map"s) {
                auto res = GenerateMapToSvg();
                arr.emplace_back(json::Builder{}.StartDict()
                                         .Key("request_id"s).Value(stat_map.at("id").AsInt())
                                         .Key("map"s).Value(res)
                                         .EndDict().Build().GetRoot());
            } else if (stat_map.at("type"s).AsString() == "Route"s) {
                const auto from = stat_map.at("from"s).AsString();
                const auto to = stat_map.at("to"s).AsString();

                auto res = t_router_.GetRoute(from, to);

                if (res) {
                    const auto &res_val = res.value();
                    double total_time = 0;
                    Array items;
                    items.reserve(res_val.size());

                    for (const auto &val: res_val) {
                        if (std::holds_alternative<BusItem>(val)) {
                            const auto edge_data = std::get<BusItem>(val);
                            total_time += edge_data.time;
                            items.emplace_back(json::Builder{}.StartDict()
                                                       .Key("bus"s).Value(edge_data.name.data())
                                                       .Key("span_count"s).Value(edge_data.span)
                                                       .Key("time"s).Value(edge_data.time)
                                                       .Key("type"s).Value("Bus"s)
                                                       .EndDict().Build().GetRoot());
                        } else {
                            const auto edge_data = std::get<WaitItem>(val);
                            total_time += edge_data.time;
                            items.emplace_back(json::Builder{}.StartDict()
                                                       .Key("stop_name"s).Value(edge_data.name.data())
                                                       .Key("time"s).Value(edge_data.time)
                                                       .Key("type"s).Value("Wait"s)
                                                       .EndDict().Build().GetRoot());
                        }
                    }

                    arr.emplace_back(json::Builder{}.StartDict()
                                             .Key("items").Value(items)
                                             .Key("request_id"s).Value(stat_map.at("id").AsInt())
                                             .Key("total_time").Value(total_time)
                                             .EndDict().Build().GetRoot());

                } else {
                    arr.emplace_back(json::Builder{}.StartDict()
                                             .Key("request_id"s).Value(stat_map.at("id").AsInt())
                                             .Key("error_message"s).Value("not found"s)
                                             .EndDict().Build().GetRoot());
                }
            }
        }

        if (!arr.empty()) {
            std::stringstream strm;
            json::Print(Document{arr}, strm);
            return strm.str();
            //auto s = strm.str();
            //std::cout << s << std::endl;
        }
        return ""s;
    }

    std::string JsonRequestProcessor::GenerateMapToSvg() {

        const auto &all_buses = db_.GetAllBuses();
        //auto all_stops = db_.GetAllStopWBusses(all_buses);
        auto all_stops = db_.GetAllStopWBusses();

        renderer_.AddBusLines(all_buses, all_stops);

        std::ostringstream map_out;

        renderer_.RenderMap(map_out);
        return map_out.str();
    }


    json::Document LoadJSONString(const std::string &s) {
        std::istringstream strm(s);
        return json::Load(strm);
    }

    json::Document LoadJSONStream(std::istream &input) {
        return json::Load(input);
    }


    std::string Print(const json::Node &node) {
        std::ostringstream out;
        Print(json::Document{node}, out);
        return out.str();
    }

    void PushInputJsonToRH(Document &json_doc, JsonRequestProcessor &rh) {
        rh.AddRequests(json_doc);
    }

}