#include "request_handler.h"

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer) :
	db_(db), renderer_(renderer)
{
}

 // Возвращает информацию о маршруте (запрос Bus)
std::optional<json::Dict> RequestHandler::GetBusStat(const std::string_view bus_name) const {
	if (db_.CheckBus(bus_name)) {
		json::Dict res;
		res["curvature"] = db_.GetBussesByStop(bus_name).curvation;
		res["route_length"] = db_.GetBussesByStop(bus_name).route_length;
		res["stop_count"] = static_cast<int>(db_.WatchAtBus(bus_name).path_stops.size());
		res["unique_stop_count"] = db_.GetBussesByStop(bus_name).unique_stops;
		return res;
	}
	return {};
}

// Возвращает маршруты, проходящие через stop_name
const json::Array RequestHandler::GetBusesByStop(const std::string_view stop_name) const {
	const auto temp_set = db_.GetStopData(stop_name);
	json::Array res;
	for (const auto stop : temp_set) {
		res.push_back(std::string(stop));
	}
	return res;
}

