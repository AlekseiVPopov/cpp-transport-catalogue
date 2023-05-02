

#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream &operator<<(std::ostream &os, const StrokeLineCap &cap) {
        switch (cap) {
            case StrokeLineCap::BUTT:
                os << "butt";
                break;
            case StrokeLineCap::ROUND:
                os << "round";
                break;
            case StrokeLineCap::SQUARE:
                os << "square";
                break;
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const StrokeLineJoin &join) {
        switch (join) {
            case StrokeLineJoin::ARCS:
                os << "arcs";
                break;
            case StrokeLineJoin::BEVEL:
                os << "bevel";
                break;
            case StrokeLineJoin::MITER:
                os << "miter";
                break;
            case StrokeLineJoin::MITER_CLIP:
                os << "miter-clip";
                break;
            case StrokeLineJoin::ROUND:
                os << "round";
                break;
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const Color &color) {
        std::ostringstream out;
        std::visit(ColorSelector{out}, color);
        os << out.str();
        return os;
    }

    void Object::Render(const RenderContext &context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

// ---------- Circle ------------------

    Circle &Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle &Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }



// ---------- Polyline ------------------

    Polyline &Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << "<polyline points=\""sv;

        bool first = true;
        for (const auto &p: points_) {
            auto x = p.x;
            auto y = p.y;
            if (first) {
                out << x << ","sv << y;
                first = false;
            } else {
                out << " "sv << x << ","sv << y;
            }
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

// ---------- Text ------------------

    Text &Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text &Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text &Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text &Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text &Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text &Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << "<text"sv;
        RenderAttrs(out);
        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y
            << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y
            << "\" font-size=\""sv << font_size_ << "\"";

        if (font_family_) {
            out << " font-family=\""sv << *font_family_ << "\""sv;
        }
        if (font_weight_) {
            out << " font-weight=\""sv << *font_weight_ << "\""sv;
        }

        out << ">"sv << data_ << "</text>"sv;
    }

    void DocumentLayer::AddPtr(std::unique_ptr<Object> &&obj) {
        objects_.emplace_back(std::move(obj));
    }

    void DocumentLayer::Render(std::ostream &out) const {
//        out << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << std::endl;
//        out << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)" << std::endl;
        RenderContext context(out, 2);
        for (const auto &obj: objects_) {
            obj->Render(context.Indented());
        }
//        out << "</svg>" << std::endl;
    }


    void Document::AddPtr(std::unique_ptr<Object> &&obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream &out) const {
        out << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << std::endl;
        out << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)" << std::endl;


        for (const auto &layer : layers_list_) {
            layer.Render(out);
        }

        RenderContext context(out, 2);

        for (const auto &obj: objects_) {
            obj->Render(context.Indented());
        }
        out << "</svg>" << std::endl;
    }

    DocumentLayer &Document::AddLayer() {
        return layers_list_.emplace_back();
    }


}  // namespace svg