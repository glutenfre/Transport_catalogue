#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator << (std::ostream& out, const StrokeLineCap& t)
    {
        switch (t) {
        case StrokeLineCap::BUTT: return (out << "butt"s);
        case StrokeLineCap::ROUND: return (out << "round"s);
        case StrokeLineCap::SQUARE: return (out << "square"s);
        }
        return (out);
    }

    std::ostream& operator << (std::ostream& out, const StrokeLineJoin& t)
    {
        switch (t) {
        case StrokeLineJoin::ARCS: return (out << "arcs"s);
        case StrokeLineJoin::BEVEL: return (out << "bevel"s);
        case StrokeLineJoin::MITER: return (out << "miter"s);
        case StrokeLineJoin::MITER_CLIP: return (out << "miter-clip"s);
        case StrokeLineJoin::ROUND: return (out << "round"s);
        }
        return (out);
    }

    // ---------- Object ------------------

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
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

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "  <polyline points=\""sv;
        bool f = false;
        for (Point xy : points_) {
            if (f) {
                out << " ";
            }
            f = true;
            out << xy.x << "," << xy.y;

        }
        out << "\" "sv;
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

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        std::string res;
        for (auto c : data) {
            if (c == '"') {
                res += "&quot;";
            }
            else if (c == '\'') {
                res += "&apos;";
            }
            else if (c == '<') {
                res += "&lt;";
            }
            else if (c == '>') {
                res += "&gt;";
            }
            else if (c == '&') {
                res += "&amp;";
            }
            else {
                res += c;
            }
        }
        data_ = std::move(res);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "  <text ";
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x
            << "\" y=\""sv << pos_.y 
            << "\" dx=\"" << offset_.x 
            << "\" dy=\"" << offset_.y 
            << "\" font-size=\""sv << size_ 
            << "\"";
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\"";
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\"";
        }
        out << ">" << data_ << "</text>";
    }


    // ---------- Document ------------------

        // Добавляет в svg-документ объект-наследник svg::Object
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        ptr_all_objects_.push_back(std::move(obj));
    }

    // Выводит в ostream svg-представление документа
    void Document::Render(std::ostream& out) const {
        const RenderContext& context(out);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (long unsigned int i = 0; i < ptr_all_objects_.size(); i++) {
            ptr_all_objects_.at(i)->Render(context);
            //out << std::endl;
        }
        out << "</svg>"sv;
    }

}  // namespace svg