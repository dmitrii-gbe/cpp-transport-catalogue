#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "transport_catalogue.h"
#include "geo.h"

namespace transport_catalogue {
    namespace detail {
        const std::string GetFlag(const std::string& s);
    }
    namespace input_reader {        
std::vector<std::string> ReadQuery();

const std::vector<std::pair<std::string, int>> ParseDistances(const std::string& s, size_t position);

std::tuple<Stop, size_t> ParseStop(const std::string& s, const size_t name_position);

const Bus ParseBus(std::string&& s, const TransportCatalogue& tc);

void ParseQueryToAdd(TransportCatalogue& tc, std::vector<std::string>&& query);
    }
}

namespace transport_catalogue {
    namespace response_output {
        
void ParseQueryToRespond(const TransportCatalogue& tc, std::vector<std::string>&& query);

void BusOutput(const std::string& name, const TransportCatalogue& tc, std::ostream& out);

void StopOutput(const std::string& name, const TransportCatalogue& tc, std::ostream& out);
    }
}