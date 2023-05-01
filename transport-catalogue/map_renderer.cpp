#include "map_renderer.h"

#include <utility>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace renderer {


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

    void MapRenderer::AddSettings(RenderSettings rs) {
        settings_ = std::move(rs);
    }

    RenderSettings &MapRenderer::GetSettings() {
        return settings_;
    }


}