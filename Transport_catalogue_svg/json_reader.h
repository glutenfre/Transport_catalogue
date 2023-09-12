#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

#include <iostream>
#include <algorithm>
#include <vector>


class JsonReader {
public:
    JsonReader(transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer, json::Document document);

	void OutputRequestedData(std::ostream& out);

    std::string GetColor(renderer::ColorSetting color_setting) const ;

    std::pair<svg::Text, svg::Text> SetText(svg::TextSetting text_setting,
        svg::TextSetting under_text_setting,
        svg::Point coord) const;

    void RenderPolyline(std::deque<Bus> temp_deque_busses,
        renderer::SphereProjector sphere_projector,
        svg::Document& res) const;

    void RenderBusName(std::deque<Bus> temp_deque_busses,
        renderer::SphereProjector sphere_projector,
        svg::Document& res) const;

    void RenderCircles(std::deque<Stop> temp_deque_stops,
        renderer::SphereProjector sphere_projector,
        svg::Document& res) const;

    void RenderStopName(std::deque<Stop> temp_deque_stops,
        renderer::SphereProjector sphere_projector,
        svg::Document& res) const;

    svg::Document RenderMap() const;

private:
    transport_catalogue::TransportCatalogue& db_;
    renderer::MapRenderer renderer_;
    json::Document document_;
};

json::Document WorkWithJsonInput(std::istream& json_input_data);

renderer::ColorSetting SetColor(const json::Node& node);

renderer::LineSettings ReadLineSettings(json::Document document);

transport_catalogue::TransportCatalogue ConvertJsonToCatalogue(json::Document& document);
