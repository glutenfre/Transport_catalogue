#pragma once

#include "geo.h"

#include <string>
#include <vector>

struct Stop {
    std::string name;
    geo::Coordinates xy;
};

struct Bus {
    std::string name;
    std::vector<Stop*> path_stops;
    bool is_roundtrip;
};

struct BusData {
    int unique_stops;
    int route_length;
    double curvation;
};

struct Request {
    std::string comand_type;
    std::string name;
    std::string rest_data;
    bool dim_flag = false;
};