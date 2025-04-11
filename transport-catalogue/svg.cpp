#include "svg.h"

namespace svg {

using namespace std::literals;

void PrintColor(std::ostream& out, std::monostate) {
    out << "none"sv;
}

void PrintColor(std::ostream& out, const std::string& str) {
    out << str;
}

void PrintColor(std::ostream& out, const Rgb& rgb) {
    out << "rgb("sv
        << static_cast<int>(rgb.red) << ","sv
        << static_cast<int>(rgb.green) << ","sv
        << static_cast<int>(rgb.blue) << ")"sv;
}

void PrintColor(std::ostream& out, const Rgba& rgba) {
    out << "rgba("sv
        << static_cast<int>(rgba.red) << ","sv
        << static_cast<int>(rgba.green) << ","sv
        << static_cast<int>(rgba.blue) << ","sv
        << rgba.opacity << ")"sv;
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap){
    switch (line_cap)
    {
    case StrokeLineCap::BUTT : return out << "butt";
    case StrokeLineCap::ROUND : return out << "round";
    case StrokeLineCap::SQUARE : return out << "square";
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
    switch (line_join) {
        case StrokeLineJoin::ARCS: return out << "arcs";
        case StrokeLineJoin::BEVEL: return out << "bevel";
        case StrokeLineJoin::MITER: return out << "miter";
        case StrokeLineJoin::MITER_CLIP: return out << "miter-clip";
        case StrokeLineJoin::ROUND: return out << "round";
    }
    return out;
}



std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit([&out](const auto& value) {
        PrintColor(out, value);
    }, color);

    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

template<typename Owner>
void PathProps<Owner>::RenderAttrs(std::ostream& out) const {
    if (fillColor_) out << " fill=\"" << *fillColor_ << "\"";
    if (strokeColor_) out << " stroke=\"" << *strokeColor_ << "\"";
    if (width_) out << " stroke-width=\"" << *width_ << "\"";
    if (lineCap_) out << " stroke-linecap=\"" << *lineCap_ << "\"";
    if (lineJoin_) out << " stroke-linejoin=\"" << *lineJoin_ << "\"";
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
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.emplace_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\"";
    for (size_t i = 0; i < points_.size(); ++i) {
        if (i > 0) out << " ";
        out << points_[i].x << "," << points_[i].y;
    }
    out << "\"";
    RenderAttrs(out);
    out << " />";
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
    font_size_ = size;
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

Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv
        << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv
        << " font-size=\""sv << font_size_ << "\""sv;

    if (!font_family_.empty())
        out << " font-family=\""sv << font_family_ << "\""sv;
    if (!font_weight_.empty())
        out << " font-weight=\""sv << font_weight_ << "\""sv;

    RenderAttrs(out);
    out << ">"sv << EscapeText(data_) << "</text>"sv;
}

std::string Text::EscapeText(const std::string& data) {
    std::string result;
    for (char c : data) {
        switch (c) {
            case '"': result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '&': result += "&amp;"; break;
            default: result += c; break;
        }
    }
    return result;
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    RenderContext ctx(out, 2);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    for (const auto& obj : objects_) {
        obj->Render(ctx);
    }

    out << "</svg>"sv;
}

}  // namespace svg