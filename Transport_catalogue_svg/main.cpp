#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "svg.h"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

int main() {
    //json::Document document = WorkWithJsonInput(std::cin);
    std::ifstream json_input_data("test1.txt");

    json::Document document = WorkWithJsonInput(json_input_data);
    renderer::MapRenderer map_renderer(ReadLineSettings(document));
    transport_catalogue::TransportCatalogue catalogue = ConvertJsonToCatalogue(document);
    JsonReader json_reader(catalogue, map_renderer, document);

    
    
    std::ofstream fout("results1.txt");

    json_reader.OutputRequestedData(std::cout);

    return 0;

}