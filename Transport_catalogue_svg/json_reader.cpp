#include "json_reader.h"

JsonReader::JsonReader(transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer, json::Document document) :
	db_(db), renderer_(renderer), document_(document)
{}

json::Document WorkWithJsonInput(std::istream& json_input_data) {
	json::Document doc = json::Load(json_input_data);
	return doc;
}

renderer::ColorSetting SetColor(const json::Node& node) {
	renderer::ColorSetting res;
	if (node.IsString()) {
		res = node.AsString();
	}
	else if (node.IsArray()) {
		json::Array temp_array = node.AsArray();
		if (temp_array.size() == 3) {
			renderer::RGB temp_rgb = {
				temp_array.at(0).AsInt(),
				temp_array.at(1).AsInt(),
				temp_array.at(2).AsInt() };
			res = temp_rgb;
		}
		else if (temp_array.size() == 4) {
			renderer::RGBA temp_rgba = {
				temp_array.at(0).AsInt(),
				temp_array.at(1).AsInt(),
				temp_array.at(2).AsInt(),
				temp_array.at(3).AsDouble() };
			res = temp_rgba;
		}
	}
	return res;
}

renderer::LineSettings ReadLineSettings(json::Document document) {
	renderer::LineSettings res;

	const json::Dict render_settings = document.GetRoot().AsMap().at("render_settings").AsMap();

	for (const auto& type_info : render_settings) {
		if (type_info.first == "width") {
			res.width = type_info.second.AsDouble();
		}
		else if (type_info.first == "height") {
			res.height = type_info.second.AsDouble();
		}
		else if (type_info.first == "padding") {
			res.pading = type_info.second.AsDouble();
		}
		else if (type_info.first == "line_width") {
			res.line_width = type_info.second.AsDouble();
		}
		else if (type_info.first == "stop_radius") {
			res.stop_radius = type_info.second.AsDouble();
		}
		else if (type_info.first == "bus_label_font_size") {
			res.bus_label_font_size = type_info.second.AsInt();
		}
		else if (type_info.first == "bus_label_offset") {
			res.bus_label_offset.first = type_info.second.AsArray().at(0).AsDouble();
			res.bus_label_offset.second = type_info.second.AsArray().at(1).AsDouble();
		}
		else if (type_info.first == "stop_label_font_size") {
			res.stop_label_font_size = type_info.second.AsInt();
		}
		else if (type_info.first == "stop_label_offset") {
			res.stop_label_offset.first = type_info.second.AsArray().at(0).AsDouble();
			res.stop_label_offset.second = type_info.second.AsArray().at(1).AsDouble();
		}
		else if (type_info.first == "underlayer_color") {
			res.underlayer_color = SetColor(type_info.second);
		}
		else if (type_info.first == "underlayer_width") {
			res.underlayer_width = type_info.second.AsDouble();
		}
		else if (type_info.first == "color_palette") {
			json::Array temp_array = type_info.second.AsArray();
			std::vector<renderer::ColorSetting> temp_vector;
			for (const auto& temp_color : temp_array) {
				temp_vector.push_back(SetColor(temp_color));
			}
			res.color_palette = temp_vector;
		}

	}
	return res;
}

transport_catalogue::TransportCatalogue ConvertJsonToCatalogue(json::Document& document) {
	transport_catalogue::TransportCatalogue db_;
	json::Array base_requests = document.GetRoot().AsMap().at("base_requests").AsArray();

	std::sort(base_requests.begin(), base_requests.end(), [](const auto& lhs, const auto& rhs) {
		return lhs.AsMap().at("type").AsString() > rhs.AsMap().at("type").AsString();
		});

	for (const auto request : base_requests) {
		json::Dict temp_dict = request.AsMap();
		if (temp_dict.at("type").AsString() == "Stop") {
			db_.AddStop(temp_dict.at("name").AsString(), temp_dict.at("latitude").AsDouble(), temp_dict.at("longitude").AsDouble());
		}
		else {
			std::vector<std::string_view> temp_vec;
			for (const auto& temp_node : temp_dict.at("stops").AsArray()) {
				temp_vec.push_back(temp_node.AsString());//не нравится мне это место, здесь ведь создается стринг вью на временную строку
			}
			db_.AddBus(temp_dict.at("name").AsString(), temp_dict.at("is_roundtrip").AsBool(), temp_vec);
		}
	}

	for (const auto request : base_requests) {
		json::Dict temp_dict = request.AsMap();
		if (temp_dict.at("type").AsString() == "Stop") {
			if (temp_dict.count("road_distances") > 0) {
				json::Dict temp_dist = temp_dict.at("road_distances").AsMap();
				for (const auto& temp_pair : temp_dist) {
					db_.AddDimmensions(temp_dict.at("name").AsString(), temp_pair.first, temp_pair.second.AsInt());
				}
			}
			else {
				db_.AddLastStop(temp_dict.at("name").AsString());
			}
			
		}
	}
	return db_;
}

std::string JsonReader::GetColor(renderer::ColorSetting color_setting) const {
	if (std::holds_alternative< renderer::RGB >(color_setting)) {
		renderer::RGB temp_rgb = std::get<renderer::RGB>(color_setting);
		std::string temp_string;
		std::ostringstream buf;
		buf << "rgb("s
			<< temp_rgb.r << ","s
			<< temp_rgb.g << ","s
			<< temp_rgb.b
			<< ")"s;
		return std::move(buf.str());
	}
	else if (std::holds_alternative< renderer::RGBA >(color_setting)) {
		renderer::RGBA temp_rgba = std::get<renderer::RGBA>(color_setting);
		std::string temp_string;
		std::ostringstream buf;
		buf << "rgba("s
			<< temp_rgba.r << ","s
			<< temp_rgba.g << ","s
			<< temp_rgba.b << ","s
			<< temp_rgba.a
			<< ")"s;
		return std::move(buf.str());
	}
	else {
		return std::move(std::get<std::string>(color_setting));
	}
}

std::pair<svg::Text, svg::Text> JsonReader::SetText(svg::TextSetting text_setting,
	svg::TextSetting under_text_setting,
	svg::Point coord) const {
	svg::Text temp_text;

	temp_text
		.SetData(std::string(text_setting.data_))
		.SetFillColor(std::string(text_setting.fill_color_))
		.SetFontFamily("Verdana"s)
		.SetFontSize(text_setting.size_)
		.SetFontWeight(std::string(text_setting.font_weight_))
		.SetOffset(text_setting.offset_)
		.SetPosition(coord);
	//.SetStrokeColor()
	//.SetStrokeLineCap()
	//.SetStrokeLineJoin()
	//.SetStrokeWidth();

	svg::Text temp_under_text;

	temp_under_text
		.SetData(std::string(under_text_setting.data_))
		.SetFillColor(std::string(under_text_setting.fill_color_))
		.SetFontFamily("Verdana"s)
		.SetFontSize(under_text_setting.size_)
		.SetOffset(under_text_setting.offset_)
		.SetPosition(coord)
		.SetStrokeColor(under_text_setting.stroke_color_)
		.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
		.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
		.SetStrokeWidth(under_text_setting.stroke_width_);

	if (!under_text_setting.font_weight_.empty()) {
		temp_under_text.SetFontWeight(std::string(under_text_setting.font_weight_));
	}
	if (!text_setting.font_weight_.empty()) {
		temp_text.SetFontWeight(std::string(text_setting.font_weight_));
	}

	return { temp_text, temp_under_text };
}

void JsonReader::RenderPolyline(std::deque<Bus> temp_deque_busses,
	renderer::SphereProjector sphere_projector,
	svg::Document& res) const {
	int j = 0;
	for (const auto& temp_bus : temp_deque_busses) {
		if (!temp_bus.path_stops.empty()) {

			svg::Polyline polyline;

			for (const auto& temps_stop : temp_bus.path_stops) {
				polyline.AddPoint(sphere_projector(temps_stop->xy));
			}

			polyline.SetStrokeColor(GetColor(renderer_.GetColorFromPalette(j)));

			polyline.SetStrokeWidth(renderer_.GetLineWidth())
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			res.Add(polyline);

			j++;
		}
	}
}

void JsonReader::RenderBusName(std::deque<Bus> temp_deque_busses, renderer::SphereProjector sphere_projector, svg::Document& res) const {
	int j = 0;
	for (const auto& temp_bus : temp_deque_busses) {
		if (!temp_bus.path_stops.empty()) {
			std::string s = "bold"s;

			svg::TextSetting text_setting;
			//text_setting.pos_ = svg::Point(temp_bus.path_stops.at(0)->xy.lat, temp_bus.path_stops.at(0)->xy.lng);
			text_setting.offset_ = svg::Point(renderer_.GetBusLabelOffset());
			text_setting.size_ = renderer_.GetBusLabelFontSize();
			text_setting.data_ = temp_bus.name;
			text_setting.fill_color_ = GetColor(renderer_.GetColorFromPalette(j));
			text_setting.font_weight_ = s;

			svg::TextSetting under_text_setting;
			//under_text_setting.pos_ = svg::Point(temp_bus.path_stops.at(0)->xy.lat, temp_bus.path_stops.at(0)->xy.lng);
			under_text_setting.offset_ = svg::Point(renderer_.GetBusLabelOffset());
			under_text_setting.size_ = renderer_.GetBusLabelFontSize();
			under_text_setting.data_ = temp_bus.name;
			under_text_setting.fill_color_ = GetColor(renderer_.GetUnderlayerColor());
			under_text_setting.stroke_color_ = GetColor(renderer_.GetUnderlayerColor());
			under_text_setting.stroke_width_ = renderer_.GetUnderlayerWidth();
			under_text_setting.font_weight_ = s;


			std::pair<svg::Text, svg::Text> text_data = SetText(text_setting, under_text_setting,
				sphere_projector({ temp_bus.path_stops.at(0)->xy.lat, temp_bus.path_stops.at(0)->xy.lng }));

			res.Add(text_data.second);
			res.Add(text_data.first);

			if (!temp_bus.is_roundtrip) {
				if (temp_bus.path_stops.at(0) != temp_bus.path_stops.at(temp_bus.path_stops.size() / 2)) {
					text_data = SetText(text_setting, under_text_setting,
						sphere_projector({ temp_bus.path_stops.at(temp_bus.path_stops.size() / 2)->xy.lat,
						temp_bus.path_stops.at(temp_bus.path_stops.size() / 2)->xy.lng }));
					res.Add(text_data.second);
					res.Add(text_data.first);
				}

			}
			j++;
		}
	}
}

void JsonReader::RenderCircles(std::deque<Stop> temp_deque_stops,
	renderer::SphereProjector sphere_projector,
	svg::Document& res) const {

	for (const auto& temp_stop : temp_deque_stops) {
		if (!db_.GetStopData(temp_stop.name).empty()) {
			svg::Circle temp_circle;
			temp_circle.SetCenter(sphere_projector(temp_stop.xy))
				.SetRadius(renderer_.GetStopRadius())
				.SetFillColor("white");
			res.Add(temp_circle);
		}
	}
}

void JsonReader::RenderStopName(std::deque<Stop> temp_deque_stops,
	renderer::SphereProjector sphere_projector,
	svg::Document& res) const {
	for (const auto& temp_stop : temp_deque_stops) {
		if (!db_.GetStopData(temp_stop.name).empty()) {
			svg::TextSetting text_setting;
			text_setting.offset_ = svg::Point(renderer_.GetStopLabelOffset());
			text_setting.size_ = renderer_.GetStopLabelFontSize();
			text_setting.data_ = temp_stop.name;
			text_setting.fill_color_ = "black";

			svg::TextSetting under_text_setting;
			under_text_setting.offset_ = svg::Point(renderer_.GetStopLabelOffset());
			under_text_setting.size_ = renderer_.GetStopLabelFontSize();
			under_text_setting.data_ = temp_stop.name;
			under_text_setting.fill_color_ = GetColor(renderer_.GetUnderlayerColor());
			under_text_setting.stroke_color_ = GetColor(renderer_.GetUnderlayerColor());
			under_text_setting.stroke_width_ = renderer_.GetUnderlayerWidth();


			std::pair<svg::Text, svg::Text> text_data = SetText(text_setting, under_text_setting,
				sphere_projector(temp_stop.xy));

			res.Add(text_data.second);
			res.Add(text_data.first);
		}
	}
}

svg::Document JsonReader::RenderMap() const {
	svg::Document res;

	int j = 0;

	std::deque<Bus> temp_deque_busses = db_.WatchAtBusses();

	std::sort(temp_deque_busses.begin(), temp_deque_busses.end(), [](const auto& lhs, const auto& rhs) {
		return lhs.name < rhs.name;
		});

	std::deque<Stop> temp_deque_stops = db_.WatchAtStops();

	std::sort(temp_deque_stops.begin(), temp_deque_stops.end(), [](const auto& lhs, const auto& rhs) {
		return lhs.name < rhs.name;
		});

	std::vector<geo::Coordinates> coordinates = db_.GetCoordinates();

	renderer::SphereProjector sphere_projector(coordinates.begin(), coordinates.end(),
		renderer_.GetWidth(), renderer_.GetHeight(), renderer_.GetPadding());

	RenderPolyline(temp_deque_busses, sphere_projector, res);

	RenderBusName(temp_deque_busses, sphere_projector, res);

	RenderCircles(temp_deque_stops, sphere_projector, res);

	RenderStopName(temp_deque_stops, sphere_projector, res);

	return res;
}

void JsonReader::OutputRequestedData(std::ostream& out) {
	const json::Array stat_requests = document_.GetRoot().AsMap().at("stat_requests").AsArray();
	RequestHandler temp_handle(db_, renderer_);
	json::Array res;
	std::string tmp_s = "not found";

	for (const auto request : stat_requests) {
		json::Dict temp_dict = request.AsMap();
		if (temp_dict.at("type").AsString() == "Stop") {
			json::Dict stop_stat;
			json::Array temp_buses;
			if (!db_.CheckStop(temp_dict.at("name").AsString())) {
				stop_stat["error_message"] = tmp_s;
			}
			else {
				temp_buses = temp_handle.GetBusesByStop(temp_dict.at("name").AsString());
				stop_stat["buses"] = json::Node(temp_buses);
			}
			stop_stat["request_id"] = temp_dict.at("id").AsInt();
			res.push_back(json::Node(stop_stat));
		}
		else if (temp_dict.at("type").AsString() == "Map") {
			json::Dict map_stat;
			std::ostringstream buf;
			svg::Document svg_document = RenderMap();
			svg_document.Render(buf);
			map_stat["map"] = buf.str();
			map_stat["request_id"] = temp_dict.at("id").AsInt();
			res.push_back(json::Node(map_stat));
		}
		else {
			json::Dict bus_stat;
			if (!db_.CheckBus(temp_dict.at("name").AsString())) {
				bus_stat["error_message"] = tmp_s;
			}
			else {
				bus_stat = temp_handle.GetBusStat(temp_dict.at("name").AsString()).value();
			}
			bus_stat["request_id"] = temp_dict.at("id").AsInt();
			res.push_back(json::Node(bus_stat));
		}
	}
	json::Print(json::Document(json::Node(res)), out);

}
