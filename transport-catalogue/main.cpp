#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"



int main () {
    std::vector<std::string> queries_to_add = transport_catalogue::input_reader::ReadQuery();
    std::vector<std::string> queries_to_respond = transport_catalogue::input_reader::ReadQuery();

    transport_catalogue::TransportCatalogue tc;

    transport_catalogue::input_reader::ParseQueryToAdd(tc, std::move(queries_to_add));
    transport_catalogue::response_output::ParseQueryToRespond(tc, std::move(queries_to_respond));

    return 0;
}