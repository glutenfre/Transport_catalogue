#include "transport_catalogue.h"

#include <algorithm>
#include <iostream>

namespace transport_catalogue {

    void TransportCatalogue::AddStop(std::string_view name, double lat, double longy) {
        Stop stop_to_push;
        stop_to_push.name = name;
        stop_to_push.xy.lat = lat;
        stop_to_push.xy.lng = longy;
        stops_.push_back(stop_to_push);
        stopname_to_stop_[stops_.back().name] = &stops_.back();
        AddBusToStop(stops_.back().name);
    }

    void TransportCatalogue::AddBus(std::string_view name, bool is_it_circle, std::vector<std::string_view> stop_names) {

        Bus perm_bus;
        perm_bus.name = std::move(name);
        perm_bus.is_roundtrip = is_it_circle;
        for (size_t i = 0; i < stop_names.size(); i++) {
            perm_bus.path_stops.push_back(&GetStop(stop_names[i]));
        }
        busses_.push_back(perm_bus);
        busname_to_bus_[busses_.back().name] = &busses_.back();

        for (size_t i = 0; i < busses_.back().path_stops.size(); i++) {
            AddBusToStop(busses_.back().path_stops[i]->name, busses_.back().name);
        }

        if (!is_it_circle) {
            for (int i = stop_names.size() - 2; i >= 0; i--) {//идем с предпоследнего эл-та к первому
                GetBus(name).path_stops.push_back(&GetStop(stop_names[i]));
            }
        }

    }

    void TransportCatalogue::AddBusToStop(std::string_view stop_name, std::string_view bus_name) {
        if (busnames_to_stop_.count(stop_name) == 0) {
            busnames_to_stop_[stop_name];
        }
        busnames_to_stop_.at(stop_name).insert(bus_name);
    }

    void TransportCatalogue::AddBusToStop(std::string_view stop_name) {
        if (busnames_to_stop_.count(stop_name) == 0) {
            busnames_to_stop_[stop_name];
        }
    }

    //возвращает ссылку на эл-т
    Bus& TransportCatalogue::GetBus(std::string_view bus_name_to_find) const {
        return *busname_to_bus_.at(bus_name_to_find);
    }

    //возвращает константную ссылку на эл-т
    const Bus& TransportCatalogue::WatchAtBus(std::string_view bus_name_to_find) const {
        return const_cast<const Bus&>(*busname_to_bus_.at(bus_name_to_find));
    }

    const std::deque<Bus> TransportCatalogue::WatchAtBusses() const {
        return busses_;
    }

    Stop& TransportCatalogue::GetStop(std::string_view stop_name_to_find) const {
        return *stopname_to_stop_.at(stop_name_to_find);
    }

    const Stop& TransportCatalogue::WatchStop(std::string_view stop_name_to_find) const {
        return GetStop(stop_name_to_find);
    }

    const std::deque<Stop> TransportCatalogue::WatchAtStops() const {
        return stops_;
    }

    BusData TransportCatalogue::GetBussesByStop(std::string_view bus_name_to_find) const {
        BusData res;
        res.route_length = 0;
        res.curvation = 0;
        double perm_curvation = 0;
        Stop* stop_from = GetBus(bus_name_to_find).path_stops.at(0);
        Stop* stop_to = GetBus(bus_name_to_find).path_stops.at(0);

        std::unordered_set<std::string_view> unique_stops;

        for (size_t i = 0; i < GetBus(bus_name_to_find).path_stops.size(); i++) {
            unique_stops.insert(GetBus(bus_name_to_find).path_stops.at(i)->name);
            if (i > 0) {
                stop_from = stop_to;
            }
            stop_to = GetBus(bus_name_to_find).path_stops.at(i);
            perm_curvation += ComputeDistance(stop_from->xy, stop_to->xy);
            if (stops_dimention_.count(std::pair(stop_from, stop_to)) > 0) {
                res.route_length += GetDimmension(stop_from->name, stop_to->name);
            }
            else if (stops_dimention_.count(std::pair(stop_to, stop_from)) > 0) {
                res.route_length += GetDimmension(stop_to->name, stop_from->name);
            }
        }

        res.unique_stops = unique_stops.size();
        res.curvation = res.route_length / perm_curvation;

        return res;
    }

    bool TransportCatalogue::CheckBus(std::string_view bus_name_to_find) const {
        return busname_to_bus_.count(bus_name_to_find) > 0;
    }

    std::set<std::string_view> TransportCatalogue::GetStopData(std::string_view stop_name_to_find) const {
        return busnames_to_stop_.at(stop_name_to_find);
    }

    bool TransportCatalogue::CheckStop(std::string_view stop_name_to_find) const {
        if (busnames_to_stop_.count(stop_name_to_find) > 0) {
            return true;
        }
        return false;
    }

    void TransportCatalogue::AddDimmensions(std::string_view stop_from, std::string_view stop_to, int dimension) {
        const auto first_stop = &GetStop(stop_from);
        const auto second_stop = &GetStop(stop_to);
        stops_dimention_[std::pair(first_stop, second_stop)] = dimension;
    }

    int TransportCatalogue::GetDimmension(std::string_view stop1_name_to_find, std::string_view stop2_name_to_find) const {
        Stop* first_stop = &GetStop(stop1_name_to_find);
        Stop* second_stop = &GetStop(stop2_name_to_find);
        return stops_dimention_.at(std::pair(first_stop, second_stop));
    }

    std::vector<geo::Coordinates> TransportCatalogue::GetCoordinates() const {
        std::vector<geo::Coordinates> res;

        for (const auto& stop_data : busnames_to_stop_) {
            if (!stop_data.second.empty()) {
                res.push_back(GetStop(stop_data.first).xy);
            }
        }

        return res;
    }

    void TransportCatalogue::AddLastStop(std::string_view stop_name) {
        last_stops_.insert(stop_name);
    }

    bool TransportCatalogue::LastStop(std::string_view stop_name) const {
        return (last_stops_.count(stop_name) > 0);
    }

}//namespace Transport_catalogue
