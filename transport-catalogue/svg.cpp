#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap value){
    using namespace std::literals;
    switch (value){
        case StrokeLineCap::BUTT : 
            out << "butt"sv;
            break;

        case StrokeLineCap::ROUND :
            out << "round"sv;
            break;

        case StrokeLineCap::SQUARE :
            out << "square"sv;
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin value){
    using namespace std::literals;
    if (value == StrokeLineJoin::ARCS){
        out << "arcs"sv;
    }
    if (value == StrokeLineJoin::BEVEL){
        out << "bevel"sv;
    }
    if (value == StrokeLineJoin::MITER){
        out << "miter"sv;
    }
    if (value == StrokeLineJoin::MITER_CLIP){
        out << "miter-clip"sv;
    }
    if (value == StrokeLineJoin::ROUND){
        out << "round"sv;
    }
    return out;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point){
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points="sv << '"';
    bool q = true;
    for (const auto& point : points_){
        if (q){
            out << point.x << ","sv << point.y;
            q = false; 
        }
        else {
            out << " "sv << point.x << "," << point.y;
        }
    } 
    out << '"';
    RenderAttrs(context.out);
    out << " />"sv;
}

// ---------- Text ------------------

    Text& Text::SetPosition(Point pos){
        position_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset){
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size){
        font_size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family){
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight){
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data){
        std::string converted_data;
        std::string quote = "'";
        for (char& c : data){
            if (c == '"'){
                converted_data += "&quot;";
            }
            else if (c == quote[0]){
                converted_data += "&apos;" ;
            }
            else if (c == '<'){
                converted_data += "&lt;" ;
            }
            else if (c == '>'){
                converted_data += "&gt;" ;
            }
            else if (c == '&'){
                converted_data += "&amp;" ;
            }
            else {
                converted_data.push_back(c);
            }
        }
        data_ = converted_data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x="sv << '"' << position_.x << '"';
        out << " y="sv << '"' << position_.y << '"';
        out << " dx="sv << '"' << offset_.x << '"';
        out << " dy="sv << '"' << offset_.y << '"';
        out << " font-size="sv << '"' << font_size_ << '"';
        if (!font_family_.empty()){
            out << " font-family="sv << '"' << font_family_ << '"';
        }
        if (!font_weight_.empty()){
            out << " font-weight="sv << '"' << font_weight_ << '"';
        }
        RenderAttrs(context.out);
        out << ">"sv;
        out << data_;
        out << "</text>";

    }

    void Document::AddPtr(std::unique_ptr<Object>&& object){
        storage_.push_back(std::move(object));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext context{out, 2, 2};
        for (const auto& item : storage_){
            item->Render(context);
        }
        out << "</svg>" << std::endl;
    }

    std::ostream& operator<<(std::ostream& out, Rgb rgb){
        out << "rgb(" << +rgb.red << "," << +rgb.green << "," << +rgb.blue << ")";
        return out;
    }

    std::ostream& operator<<(std::ostream& out, Rgba rgba){
        out << "rgba(" << +rgba.red << "," << +rgba.green << "," << +rgba.blue << "," << +rgba.opacity << ")";
        return out;
    }

    std::ostream& operator<<(std::ostream& out, Color color){
    if(std::holds_alternative<std::monostate>(color)){
        out << "none";
    }
    if(std::holds_alternative<std::string>(color)){
        out << std::get<std::string>(color);
    }
    if(std::holds_alternative<Rgb>(color)){
        out << std::get<Rgb>(color);
    }
    if(std::holds_alternative<Rgba>(color)){
        out << std::get<Rgba>(color);
    }
    return out;
}

}  // namespace svg