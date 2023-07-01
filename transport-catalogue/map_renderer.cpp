#include "map_renderer.h"

#include <utility>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace transport_catalogue {

    using namespace std::string_literals;

    void MapRenderer::AddTransportLine(std::vector<svg::Point> &points, svg::Color color) {

        auto line = svg::Polyline();

        for (const auto &p: points) {
            line.AddPoint(p);
        }
        line.SetStrokeColor(color)
                .SetFillColor(svg::NoneColor)
                .SetStrokeWidth(settings_.line_width)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line_layer_.Add(line);
    }

    void MapRenderer::AddTransportLineName(const svg::Point &point, const std::string &text, const svg::Color color) {

        auto line_name_under = svg::Text().SetData(text)
                .SetPosition(point)
                .SetOffset(settings_.bus_label_offset)
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetStrokeColor(settings_.underlayer_color)
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        auto line_name = svg::Text().SetData(text)
                .SetPosition(point)
                .SetOffset(settings_.bus_label_offset)
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetFillColor(color);

        line_name_layer_.Add(line_name_under);
        line_name_layer_.Add(line_name);

    }

    void MapRenderer::AddTransportLineCircle(const svg::Point &point, const svg::Color color) {
        auto circle = svg::Circle().SetRadius(settings_.stop_radius)
                .SetCenter(point)
                .SetFillColor(color);
        line_stop_circle_layer_.Add(circle);

    }

    void
    MapRenderer::AddTransportLineStopName(const svg::Point &points, const std::string &name, const svg::Color color) {


        auto text_under = svg::Text().SetData(name)
                .SetPosition(points)
                .SetOffset(settings_.stop_label_offset)
                .SetFontSize(settings_.stop_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeColor(settings_.underlayer_color)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        auto text = svg::Text().SetData(name)
                .SetPosition(points)
                .SetOffset(settings_.stop_label_offset)
                .SetFontSize(settings_.stop_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFillColor(color);

        line_stop_name_layer_.Add(text_under);
        line_stop_name_layer_.Add(text);
    }

    void MapRenderer::RenderMap(std::ostream &out) const {
        document_.Render(out);
    }

    void MapRenderer::SetSettings(RenderSettings &&rs) {
        settings_ = std::forward<RenderSettings>(rs);
    }

    void MapRenderer::AddBusLines(const std::vector<const Bus *> &buses, const std::vector<const Stop *> &stops) {
        int line_color = 0;
        int line_max_color = static_cast<int>(settings_.color_palette.size());

        std::vector<geo::Coordinates> all_stops_coords;

        all_stops_coords.reserve(stops.size());

        for (const auto &stop: stops) {
            all_stops_coords.emplace_back(stop->coordinates);
        }

        const transport_catalogue::SphereProjector proj(all_stops_coords.begin(),
                                                        all_stops_coords.end(),
                                                        settings_.width,
                                                        settings_.height,
                                                        settings_.padding);


        for (const auto &bus: buses) {
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
            AddTransportLine(line_points, settings_.color_palette[line_color]);
            AddTransportLineName(*line_points.begin(), bus->bus_name, settings_.color_palette[line_color]);
            if (auto scnd_stop = std::next(line_points.begin(), (bus->stops_num + 1) / 2) - 1;
                    !bus->is_circled && scnd_stop->x != line_points.begin()->x &&
                    scnd_stop->y != line_points.begin()->y) {

                AddTransportLineName(*scnd_stop, bus->bus_name, settings_.color_palette[line_color]);
            }

            if (++line_color >= line_max_color) {
                line_color = 0;
            }
        }

        for (const auto &stop: stops) {
            svg::Point screen_coord = proj(stop->coordinates);
            AddTransportLineCircle(screen_coord, "white"s);
            AddTransportLineStopName(screen_coord, stop->stop_name, "black"s);
        }
    }

    transport_catalogue_protobuf::Color MapRenderer::SerializeColor(const svg::Color &color) const {
        transport_catalogue_protobuf::Color proto_color;

        if (std::holds_alternative<std::monostate>(color)) {
            proto_color.set_color_string("none");

        } else if (std::holds_alternative<svg::Rgb>(color)) {
            svg::Rgb rgb = std::get<svg::Rgb>(color);

            proto_color.mutable_rgb()->set_red(rgb.red);
            proto_color.mutable_rgb()->set_green(rgb.green);
            proto_color.mutable_rgb()->set_blue(rgb.blue);

        } else if (std::holds_alternative<svg::Rgba>(color)) {
            svg::Rgba rgba = std::get<svg::Rgba>(color);

            proto_color.mutable_rgba()->set_red(rgba.red);
            proto_color.mutable_rgba()->set_green(rgba.green);
            proto_color.mutable_rgba()->set_blue(rgba.blue);
            proto_color.mutable_rgba()->set_opacity(rgba.opacity);

        } else if (std::holds_alternative<std::string>(color)) {
            proto_color.set_color_string(std::get<std::string>(color));
        }
        return proto_color;
    }

    svg::Color MapRenderer::DeserializeColor(const transport_catalogue_protobuf::Color &proto_color) const {
        if (proto_color.has_rgb()) {
            return svg::Rgb{static_cast<uint8_t>(proto_color.rgb().red()),
                            static_cast<uint8_t>(proto_color.rgb().green()),
                            static_cast<uint8_t>(proto_color.rgb().blue())};
        } else if (proto_color.has_rgba()) {
            return svg::Rgba{static_cast<uint8_t>(proto_color.rgba().red()),
                             static_cast<uint8_t>(proto_color.rgba().green()),
                             static_cast<uint8_t>(proto_color.rgba().blue()),
                             proto_color.rgba().opacity()};
        }
        return {proto_color.color_string()};
    }

    transport_catalogue_protobuf::RenderSettings MapRenderer::Serialize() const {
        transport_catalogue_protobuf::RenderSettings proto_render_settings;

        proto_render_settings.set_width(settings_.width);
        proto_render_settings.set_height(settings_.height);
        proto_render_settings.set_padding(settings_.padding);
        proto_render_settings.set_line_width(settings_.line_width);
        proto_render_settings.set_stop_radius(settings_.stop_radius);
        proto_render_settings.set_bus_label_font_size(settings_.bus_label_font_size);

        transport_catalogue_protobuf::Point proto_bus_label_offset;
        proto_bus_label_offset.set_x(settings_.bus_label_offset.x);
        proto_bus_label_offset.set_y(settings_.bus_label_offset.y);

        *proto_render_settings.mutable_bus_label_offset() = std::move(proto_bus_label_offset);

        proto_render_settings.set_stop_label_font_size(settings_.stop_label_font_size);

        transport_catalogue_protobuf::Point proto_stop_label_offset;
        proto_stop_label_offset.set_x(settings_.stop_label_offset.x);
        proto_stop_label_offset.set_y(settings_.stop_label_offset.y);

        *proto_render_settings.mutable_stop_label_offset() = std::move(proto_stop_label_offset);
        *proto_render_settings.mutable_underlayer_color() = std::move(SerializeColor(settings_.underlayer_color));
        proto_render_settings.set_underlayer_width(settings_.underlayer_width);

        for (const auto &color: settings_.color_palette) {
            *proto_render_settings.add_color_palette() = SerializeColor(color);
        }
        return proto_render_settings;
    }

    void
    MapRenderer::Deserialize(const transport_catalogue_protobuf::RenderSettings &proto_render_settings) {
        transport_catalogue::RenderSettings render_settings;

        render_settings.width = proto_render_settings.width();
        render_settings.height = proto_render_settings.height();
        render_settings.padding =proto_render_settings.padding();
        render_settings.line_width = proto_render_settings.line_width();
        render_settings.stop_radius = proto_render_settings.stop_radius();
        render_settings.bus_label_font_size = proto_render_settings.bus_label_font_size();

        render_settings.bus_label_offset.x = proto_render_settings.bus_label_offset().x();
        render_settings.bus_label_offset.y = proto_render_settings.bus_label_offset().y();

        render_settings.stop_label_font_size = proto_render_settings.stop_label_font_size();

        render_settings.stop_label_offset.x = proto_render_settings.stop_label_offset().x();
        render_settings.stop_label_offset.y = proto_render_settings.stop_label_offset().y();

        render_settings.underlayer_color = DeserializeColor(proto_render_settings.underlayer_color());
        render_settings.underlayer_width = proto_render_settings.underlayer_width();

        render_settings.color_palette.clear();
        for (const auto &color_proto: proto_render_settings.color_palette()) {
            render_settings.color_palette.push_back(DeserializeColor(color_proto));
        }

        SetSettings(std::move(render_settings));
    }


}