#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    void Rgb::Render(std::ostream& out) const {
        out << "rgb(" << static_cast<int>(red)
            << ',' << static_cast<int>(green)
            << ',' << static_cast<int>(blue) << ')';
    }

    void Rgba::Render(std::ostream& out) const {
        out << "rgba(" << static_cast<int>(red)
            << ',' << static_cast<int>(green)
            << ',' << static_cast<int>(blue)
            << ',' << opacity << ')';
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first = false;
        for (const auto point : points_) {
            if (first) { out << ' '; }
            out << point.x << ',' << point.y;
            first = true;
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }
    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }
    Text& Text::SetFontFamily(const std::string& font_family) {
        font_family_ = font_family;
        return *this;
    }
    Text& Text::SetFontWeight(const std::string& font_weight) {
        font_weight_ = font_weight;
        return *this;
    }
    Text& Text::SetData(const std::string& data) {
        data_ = data;
        return *this;
    }
    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y;
        out << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y;
        out << "\" font-size=\""sv << size_;
        if (!font_family_.empty()) {
            out << "\" font-family=\""sv << font_family_;
        }
        if (!font_weight_.empty()) {
            out << "\" font-weight=\""sv << font_weight_;
        }
        out << "\">"sv;
        std::map<char, std::string> symbol{
            {'\"', "&quot;"},
            { '\'', "&apos;" },
            { '<', "&lt;" },
            { '>', "&gt;" },
            { '&', "&amp;" }
        };
        for (char str : data_) {
            if (symbol.count(str)) {
                out << symbol.at(str);
            }
            else {
                out << str;
            }
        }

        out << "</text>"sv;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }
    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        RenderContext ctx(out, 2, 2);
        for (const auto& object : objects_) {
            object->Render(ctx);
        }

        out << "</svg>"sv;
    }

}  // namespace svg