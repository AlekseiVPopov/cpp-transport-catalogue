#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

namespace request_handler {

    using namespace std::string_literals;

    std::optional<BusInfoResponse> RequestHandler::GetBusStat(const std::string_view &bus_name) const {
        return db_.GetBusInfo(bus_name);
    }

    std::optional<StopInfoResponse> RequestHandler::GetBusesByStop(const std::string_view &stop_name) const {
        return db_.GetStopInfo(stop_name);
    }

    void RequestHandler::AddRequests(const json::Document &json_doc) {
        try {
            const auto &doc = json_doc.GetRoot();

            for (const auto &[category, cat_value]: doc.AsMap()) {
                if (category == "base_requests"s) {
                    for (const auto &req_value: cat_value.AsArray()) {
                        const auto &command = req_value.AsMap();
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
                } else if (category == "render_settings"s && !cat_value.AsMap().empty()) {
                    ParseSettings(cat_value);
                }
            }
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    }


    void RequestHandler::ParseBaseRequests() {
        for (const auto stop_jnode: base_stops_requests_) {
            const auto &stop_map = stop_jnode->AsMap();
            InputStopInfo stop_info{stop_map.at("name"s).AsString(),
                                    {stop_map.at("latitude"s).AsDouble(),
                                     stop_map.at("longitude"s).AsDouble()}};
            parsed_stop_info_deque_.emplace_back(std::move(stop_info));

            const auto &dist_info_map = stop_map.at("road_distances"s).AsMap();
            if (!dist_info_map.empty()) {
                InputDistanceInfo dist_info{stop_map.at("name"s).AsString(), {}};
                for (const auto &[neigh_name, dist]: dist_info_map) {
                    dist_info.distance_to_neighbour.insert(std::make_pair(neigh_name, dist.AsInt()));
                }
                parsed_distance_info_deque_.emplace_back(std::move(dist_info));
            }
        }


        for (const auto bus_jnode: base_bus_requests_) {
            const auto &bus_map = bus_jnode->AsMap();
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


    void RequestHandler::ParseSettings(const Node &settings_node) {

        renderer::RenderSettings rs;

        for (const auto &[key, value]: settings_node.AsMap()) {
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
        renderer_.AddSettings(rs);
    }


    void RequestHandler::PushBaseRequest() {

        for (const auto &stop: parsed_stop_info_deque_) {
            db_.AddStop(&stop);
        }

        for (const auto &bus: parsed_bus_info_deque_) {
            db_.AddBus(&bus);
        }

        for (const auto &dist: parsed_distance_info_deque_) {
            db_.AddRealDistance(&dist);
        }
    }

    std::string RequestHandler::PushStatRequests() {

        Array arr;
        arr.reserve(1'000);

        for (const auto stat_jnode: stat_requests_) {
            const auto &stat_map = stat_jnode->AsMap();
            if (stat_map.at("type"s).AsString() == "Bus"s) {
                const auto res = db_.GetBusInfo(stat_map.at("name"s).AsString());
                if (res) {
                    const auto &res_val = res.value();
                    arr.emplace_back(Dict{
                            {"request_id"s,        stat_map.at("id").AsInt()},
                            {"curvature"s,         res_val.curvature},
                            {"route_length"s,      res_val.real_length},
                            {"stop_count"s,        res_val.stops_num},
                            {"unique_stop_count"s, res_val.uniq_stops_num}
                    });
                } else {
                    arr.emplace_back(Dict{
                            {"request_id"s,    stat_map.at("id").AsInt()},
                            {"error_message"s, "not found"s}
                    });
                }
            } else if (stat_map.at("type"s).AsString() == "Stop"s) {
                auto res = db_.GetStopInfo(stat_map.at("name"s).AsString());

                if (res) {
                    const auto &res_val = res.value();
                    Array bus_arr;
                    for (const auto &bus: res_val.buses) {
                        bus_arr.emplace_back(bus->bus_name);
                    }

                    arr.emplace_back(Dict{
                            {"request_id"s, stat_map.at("id").AsInt()},
                            {"buses"s,      bus_arr}
                    });
                } else {
                    arr.emplace_back(Dict{
                            {"request_id"s,    stat_map.at("id").AsInt()},
                            {"error_message"s, "not found"s}
                    });
                }
            } else if (stat_map.at("type"s).AsString() == "Map"s) {
                auto res = GenerateMapToSvg();
                arr.emplace_back(Dict{
                        {"request_id"s, stat_map.at("id").AsInt()},
                        {"map"s,        res}
                });

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

    std::string RequestHandler::GenerateMapToSvg() {

        auto &rs = renderer_.GetSettings();


        int line_color = 0;
        int line_max_color = static_cast<int>(rs.color_palette.size());

        const auto &all_buses = db_.GetAllBuses();

        std::vector<geo::Coordinates> all_stops_coords;
        auto all_stops = db_.GetAllStopWBusses();
        all_stops_coords.reserve(all_stops.size());

        for (const auto &stop: all_stops) {
            all_stops_coords.emplace_back(stop->coordinates);
        }

        const renderer::SphereProjector proj(all_stops_coords.begin(),
                                             all_stops_coords.end(),
                                             rs.width,
                                             rs.height,
                                             rs.padding);


        for (const auto &bus: all_buses) {
            std::vector<geo::Coordinates> current_stops_coords;
            current_stops_coords.reserve(bus->stops_num);

            for (const auto &stop: bus->stops) {
                current_stops_coords.push_back(stop->coordinates);
            }

            std::vector<svg::Point> line_points;
            line_points.reserve(bus->stops_num);

            for (const auto &geo_coord: current_stops_coords) {
                const svg::Point screen_coord = proj(geo_coord);
                line_points.emplace_back(screen_coord);
            }
            renderer_.AddTransportLine(line_points, rs.color_palette[line_color]);
            renderer_.AddTransportLineName(*line_points.begin(), bus->bus_name, rs.color_palette[line_color]);
            if (auto scnd_stop = std::next(line_points.begin(), (bus->stops_num + 1) / 2) - 1;
                    !bus->is_circled && scnd_stop->x != line_points.begin()->x &&
                    scnd_stop->y != line_points.begin()->y) {

                renderer_.AddTransportLineName(*scnd_stop, bus->bus_name, rs.color_palette[line_color]);
            }

            if (++line_color >= line_max_color) {
                line_color = 0;
            }
        }

        for (const auto &stop: all_stops) {
            svg::Point screen_coord = proj(stop->coordinates);
            renderer_.AddTransportLineCircle(screen_coord, "white"s);
            renderer_.AddTransportLineStopName(screen_coord, stop->stop_name, "black"s);
        }

        std::ostringstream map_out;

        renderer_.RenderMap(map_out);
        return map_out.str();
    }


}