#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace detail {
        const std::string GetFlag(const std::string& s);
    }
    namespace input_reader {        
std::vector<std::string> ReadQuery();

const std::vector<std::pair<std::string, int>> ParseDistances(const std::string& s, size_t position);

std::tuple<Stop, size_t> ParseStop(const std::string& s, const size_t name_position);

std::tuple<std::string, std::vector<std::string>, bool> ParseBus(std::string&& s);

void ParseQueryToAdd(TransportCatalogue& tc, std::vector<std::string>&& query);
    }
}

