#include "stat_reader.h"

namespace transport_catalogue {
    namespace response_output {
        
void BusOutput(const Bus* bus, const std::string& name, const TransportCatalogue& tc){
    if (bus == nullptr){
        std::cout << "Bus " << name << ": not found" << std::endl;
    }
    else {
        double geo_route_length = 0;
        double real_route_length = 0;
        size_t n = bus->stops.size() - 1;
        for (size_t i = 0; i + 1 != bus->stops.size(); ++i){
            geo_route_length += ComputeDistance({bus->stops[i]->latitude, bus->stops[i]->longitude}, {bus->stops[i + 1]->latitude, bus->stops[i + 1]->longitude});
            if (!bus->is_circular){
                real_route_length += tc.GetDistances({bus->stops[i], bus->stops[i + 1]}) + tc.GetDistances({bus->stops[n], bus->stops[n - 1]});
                --n;
            }
            else {
                real_route_length += tc.GetDistances({bus->stops[i], bus->stops[i + 1]});
            }
        }
        std::unordered_set<Stop*> unique_elements(bus->stops.begin(), bus->stops.end());
        std::cout << "Bus " << bus->name << ": " << (bus->is_circular == true ? bus->stops.size() : bus->stops.size() * 2 - 1) << " stops on route, " << unique_elements.size() << " unique stops, " << real_route_length << " route length, " << (bus->is_circular == true ? real_route_length / geo_route_length : real_route_length / geo_route_length / 2) << " curvature" << std::endl;
    }    
}

void StopOutput(std::optional<const std::set<std::string_view>> buses, const std::string& name){
    if (!buses.has_value()){
        std::cout << "Stop " << name << ": not found" << std::endl;
    }
    else if (buses.value().empty()){
        std::cout << "Stop " << name << ": no buses" << std::endl;
    }
    else {
        bool is_not_first_entry = false;
        std::cout << "Stop " << name << ": buses ";
        for (const auto& bus : buses.value()){
            if (is_not_first_entry == true){
                std::cout << " " << bus;
            }
            else {
                std::cout << bus;
            }
            is_not_first_entry = true;
        }
        std::cout << std::endl;
    }
}

void ParseQueryToRespond(const TransportCatalogue& tc, std::vector<std::string>&& query){
    for (const auto s : query){
        size_t position = s.find_first_of(" ", 0);
        std::string flag = s.substr(0, position);
        std::string name = s.substr(++position);
        if (flag == "Bus"){
            BusOutput(tc.FindBus(name), name, tc);
        }
        if (flag == "Stop"){
            StopOutput(tc.GetBusesForStop(name), name);
        }
    }    
}
    }
}