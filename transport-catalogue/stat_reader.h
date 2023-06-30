#pragma once

#include <iostream>
#include "transport_catalogue.h"
#include "geo.h"

namespace transport_catalogue {
    namespace response_output {
        
void ParseQueryToRespond(const TransportCatalogue& tc, std::vector<std::string>&& query);

void BusOutput(const std::string& name, const TransportCatalogue& tc, std::ostream& out);

void StopOutput(const std::string& name, const TransportCatalogue& tc, std::ostream& out);
    }
}