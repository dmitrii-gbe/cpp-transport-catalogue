#pragma once

#include <iostream>
#include "transport_catalogue.h"
#include "geo.h"

namespace transport_catalogue {
    namespace response_output {
        
void ParseQueryToRespond(const TransportCatalogue& tc, std::vector<std::string>&& query);

void BusOutput(const Bus* bus, const std::string& name, const TransportCatalogue& tc);

void StopOutput(std::optional<const std::set<std::string_view>> buses, const std::string& name);
    }
}