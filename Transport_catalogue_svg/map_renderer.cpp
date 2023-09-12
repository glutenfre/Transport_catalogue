#include "map_renderer.h"

namespace renderer {

inline const double EPSILON = 1e-6;

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

// Проецирует широту и долготу в координаты внутри SVG-изображения
svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

size_t MapRenderer::GetColorNumber() const {
	return line_set_.color_palette.size();
}

ColorSetting MapRenderer::GetColorFromPalette(size_t i) const {
	return line_set_.color_palette.at(i%(GetColorNumber()) );
}

double MapRenderer::GetLineWidth() const {
	return line_set_.line_width;
}

double MapRenderer::GetPadding() const {
	return line_set_.pading;
}

double MapRenderer::GetWidth() const {
    return line_set_.width;
}
double MapRenderer::GetHeight() const {
    return line_set_.height;
}

double MapRenderer::GetStopRadius() const {
    return line_set_.stop_radius;
}
int MapRenderer::GetBusLabelFontSize() const {
    return line_set_.bus_label_font_size;
}
std::pair<double, double> MapRenderer::GetBusLabelOffset() const {
    return line_set_.bus_label_offset;
}
int MapRenderer::GetStopLabelFontSize() const {
    return line_set_.stop_label_font_size;
}
std::pair<double, double> MapRenderer::GetStopLabelOffset() const{
    return line_set_.stop_label_offset;
}
ColorSetting MapRenderer::GetUnderlayerColor() const {
    return line_set_.underlayer_color;
}
double MapRenderer::GetUnderlayerWidth() const {
    return line_set_.underlayer_width;
}

}
