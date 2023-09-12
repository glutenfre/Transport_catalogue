#pragma once

#include "geo.h"
#include "domain.h"

#include <string>// указываем библиотеки в header
#include <vector>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <utility>
#include <functional>

template<>
struct std::hash< std::pair<Stop*, Stop*> > {
    size_t operator()(const std::pair<Stop*, Stop*>& my_pair) const {
        return std::hash<void*>{}(my_pair.first) + std::hash<void*>{}(my_pair.second) * 37;
    }
};

namespace transport_catalogue {

    class TransportCatalogue {
    public:

        //добавление остановки
        void AddStop(std::string_view name, double lat, double longy);
        //добавление маршрута в базу
        void AddBus(std::string_view name, bool is_it_circle, std::vector<std::string_view> stop_names);

        //поиск маршрута по имени
        Bus& GetBus(std::string_view bus_name_to_find) const ;
        const Bus& WatchAtBus(std::string_view bus_name_to_find) const;
        const std::deque<Bus> WatchAtBusses() const;

        //поиск остановки по имени
        Stop& GetStop(std::string_view stop_name_to_find) const;
        const Stop& WatchStop(std::string_view stop_name_to_find) const;
        const std::deque<Stop> WatchAtStops() const;

        //получение информации о маршруте
        BusData GetBussesByStop(std::string_view bus_name_to_find) const ;
        //поиск массива автобусов остановки по ее имени
        std::set<std::string_view> GetStopData(std::string_view stop_name_to_find) const ;

        //проверяю, существует ли автобус с таким названием
        bool CheckBus(std::string_view bus_name_to_find) const;
        //проверяю, существует ли остановка 
        bool CheckStop(std::string_view stop_name_to_find) const;

        void AddDimmensions(std::string_view stop_from, std::string_view stop_to, int dimension);
        int GetDimmension(std::string_view stop1_name_to_find, std::string_view stop2_name_to_find) const;

        void AddLastStop(std::string_view stop_name);
        bool LastStop(std::string_view stop_name) const;

        std::vector<geo::Coordinates> GetCoordinates() const ;

    private:

        void AddBusToStop(std::string_view stop_name, std::string_view bus_name);
        void AddBusToStop(std::string_view stop_name);

        //здесь хранятся остановки - имя и две координаты
        std::deque< Stop > stops_;
        std::unordered_map< std::string_view, Stop* > stopname_to_stop_;

        //здесь хранятся автобусы - имя и все остановки
        std::deque< Bus > busses_;
        std::unordered_map< std::string_view, Bus* > busname_to_bus_;

        //это массив указателей на название остановки и вектор указателей на проезжающие ее автобусы
        std::unordered_map< std::string_view, std::set<std::string_view> > busnames_to_stop_;

        //контейнер, содержащий пару остановок и расстояние между ними
        std::unordered_map<std::pair<Stop*, Stop*>, int> stops_dimention_;

        //информация о конечных остановках
        std::unordered_set<std::string_view> last_stops_;

    };
}//namespace Transport_catalogue
