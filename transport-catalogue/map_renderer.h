#pragma once

#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <utility>
#include <vector>
/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */



namespace renderer {

    using namespace std::string_literals;

    inline const double EPSILON = 1e-6;

    struct RenderSettings {

        double width = 1200.0;
        double height = 1200.0;

        double padding = 50.0;

        double line_width = 14.0;
        double stop_radius = 5.0;

        uint32_t bus_label_font_size = 20;
        //std::vector<double> bus_label_offset = {7.0, 15.0};
        svg::Point bus_label_offset = {7.0, 15.0};

        uint32_t stop_label_font_size = 20;
        //std::vector<double> stop_label_offset = {7.0, -3.0};
        svg::Point stop_label_offset = {7.0, -3.0};

        svg::Color underlayer_color = svg::Rgba{255, 255, 255, 0.85};
        double underlayer_width = 3.0;

        std::vector<svg::Color> color_palette = {"green"s, svg::Rgb{255, 160, 0}, "red"s};
    };


    class SphereProjector {
    public:
        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }

        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template<typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding)
                : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            } else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            } else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                    (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };


    class MapRenderer {
    public:

//        template<typename DrawableIterator>
//        void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer &target) {
//            for (auto it = begin; it != end; ++it) {
//                (*it)->Draw(target);
//            }
//        }
//
//        template<typename Container>
//        void DrawPicture(const Container &container, svg::ObjectContainer &target) {
//            using namespace std;
//            DrawPicture(begin(container), end(container), target);
//        }

//        // Выполняет линейную интерполяцию значения от from до to в зависимости от параметра t
//        uint8_t Lerp(uint8_t from, uint8_t to, double t) {
//            return static_cast<uint8_t>(std::round((to - from) * t + from));
//        }
//
//        // Выполняет линейную интерполяцию Rgb цвета от from до to в зависимости от параметра t
//        svg::Rgb Lerp(svg::Rgb from, svg::Rgb to, double t) {
//            return {Lerp(from.red, to.red, t),
//                    Lerp(from.green, to.green, t),
//                    Lerp(from.blue, to.blue, t)};
//        }
//


        explicit MapRenderer(svg::Document &document) : document_(document),
                                                        line_layer_(document_.AddLayer()),
                                                        line_name_layer_(document_.AddLayer()),
                                                        line_stop_circle_layer_(document_.AddLayer()),
                                                        line_stop_name_layer_(document_.AddLayer()) {}

        void AddTransportLine(std::vector<svg::Point> &points, svg::Color color);

        void AddTransportLineName(const svg::Point &point, const std::string &text, const svg::Color color);

        void AddTransportLineCircle(const svg::Point &point, const svg::Color color);

        void AddTransportLineStopName(const svg::Point &points, const std::string &name, const svg::Color color);

        void RenderMap(std::ostream &out) const;

        void AddSettings(RenderSettings rs);

        RenderSettings &GetSettings();


    private:
        RenderSettings settings_;
        svg::Document &document_;
        svg::DocumentLayer &line_layer_;
        svg::DocumentLayer &line_name_layer_;
        svg::DocumentLayer &line_stop_circle_layer_;
        svg::DocumentLayer &line_stop_name_layer_;

    };


}