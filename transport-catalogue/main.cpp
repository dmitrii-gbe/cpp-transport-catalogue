#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "transport_router.h"
#include "serialization.h"

#include <iostream>
#include <iomanip>
#include <istream>
#include <ostream>
#include <fstream>
#include <sstream>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);
    
    if (mode == "make_base"sv) {

        json::Document queries = json_reader::GetQuery(std::cin);

        myproto::MakeBase(queries);

    } else if (mode == "process_requests"sv) {

        json::Document request_queries = json_reader::GetQuery(std::cin);

        myproto::ProcessRequests(request_queries.GetRoot().AsMap().at("serialization_settings").AsMap().at("file").AsString(), request_queries);

    } else {
        PrintUsage();
        return 1;
    }
}
