#pragma once

#include "transport_catalogue.h"
#include "svg.h"
#include "json.h"
#include "map_renderer.h"
#include "geo.h"

#include <optional>
#include <string>
#include <vector>
#include <variant>
#include <sstream>

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<json::Dict> GetBusStat(const std::string_view bus_name) const;

    // Возвращает маршруты, проходящие через stop_name
    //const std::unordered_set<Bus*>* GetBusesByStop(const std::string_view& stop_name) const;
    const json::Array GetBusesByStop(const std::string_view stop_name) const;

private:
    //RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport_catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};


