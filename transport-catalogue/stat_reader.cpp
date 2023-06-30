#include <iomanip>
#include "stat_reader.h"

namespace transport_catalogue {
    namespace response_output {
        
void BusOutput(const std::string& name, const TransportCatalogue& tc, std::ostream& out){
    const Bus* bus = tc.FindBus(name);
    if (bus == nullptr){
        out << "Bus " << name << ": not found" << std::endl;
    }
    else {
        auto geo_and_real_route_lengths = tc.CalculateRouteLength(bus);
        double geo_route_length = geo_and_real_route_lengths.first;
        double real_route_length = geo_and_real_route_lengths.second;
        std::unordered_set<Stop*> unique_elements(bus->stops.begin(), bus->stops.end());
        out << "Bus " << bus->name << ": " << (bus->is_circular == true ? bus->stops.size() : bus->stops.size() * 2 - 1) << " stops on route, " << unique_elements.size() << " unique stops, " << real_route_length << " route length, " << (bus->is_circular == true ? real_route_length / geo_route_length : real_route_length / geo_route_length / 2) << " curvature" << std::endl;
    }    
}

void StopOutput(const std::string& name, const TransportCatalogue& tc, std::ostream& out){
    auto buses = tc.GetBusesForStop(name);
    if (!buses.has_value()){
        out << "Stop " << name << ": not found" << std::endl;
    }
    else if (buses.value().empty()){
        out << "Stop " << name << ": no buses" << std::endl;
    }
    else {
        bool is_not_first_entry = false;
        out << "Stop " << name << ": buses ";
        for (const auto& bus : buses.value()){
            if (is_not_first_entry == true){
                out << " " << bus;
            }
            else {
                out << bus;
            }
            is_not_first_entry = true;
        }
        out << std::endl;
    }
}

void ParseQueryToRespond(const TransportCatalogue& tc, std::vector<std::string>&& query){
    for (const auto s : query){
        size_t position = s.find_first_of(" ", 0);
        std::string flag = s.substr(0, position);
        std::string name = s.substr(++position);
        if (flag == "Bus"){
            BusOutput(name, tc, std::cout);
        }
        if (flag == "Stop"){
            StopOutput(name, tc, std::cout);
        }
    }    
}
    }
}