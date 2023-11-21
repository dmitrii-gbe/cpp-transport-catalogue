#include "transport_catalogue.h"
#include <iostream>

namespace transport_catalogue {

void TransportCatalogue::AddBus(const Bus& bus){
    buses_.push_back(bus);
    bus_names_to_bus_.emplace(prev(buses_.end())->name, &(*prev(buses_.end())));
}

void TransportCatalogue::AddStop(const Stop& stop){
    stops_.push_back(stop);
    stop_names_to_stop_.emplace(prev(stops_.end())->name, &(*prev(stops_.end())));
}

const Bus* TransportCatalogue::FindBus(const std::string& name) const {
    if (bus_names_to_bus_.count(name) == 0){
        return nullptr;
    } else {
        return bus_names_to_bus_.at(name);
    }
}

std::optional<const std::set<std::string_view>> TransportCatalogue::GetBusesForStop(const std::string& stop_query) const {
    if (stop_names_to_stop_.count(stop_query) == 0){
        return std::nullopt;
    }
    std::set<std::string_view> result;
    for (const auto& bus : buses_){
        for (const auto& stop : bus.stops){
            if (stop_query == stop->name){
                result.insert(bus.name);
            }
        }
    }
    return result;
}

void TransportCatalogue::SetDistance(const std::string& stop_name_to, const std::string& stop_name_from, const int distance){
        const std::pair<Stop*, Stop*> ptr_pair = {stop_names_to_stop_.at(stop_name_from), stop_names_to_stop_.at(stop_name_to)};
        distances_.emplace(ptr_pair, distance);
}

double TransportCatalogue::GetDistance(Stop* const from, Stop* const to) const{
    const std::pair<Stop*, Stop*> dir_pair = {from, to};
    const std::pair<Stop*, Stop*> rev_pair = {to, from};
    if (distances_.count(dir_pair) != 0){
        return distances_.at(dir_pair);
    }
    else {
        return distances_.at(rev_pair);
    }
}

std::pair<double, double> TransportCatalogue::CalculateRouteLength(const Bus* bus) const{
    double geo_route_length = 0.0;
    double real_route_length = 0.0;
    size_t n = bus->stops.size() - 1;
    for (size_t i = 0; i + 1 != bus->stops.size(); ++i){
        geo_route_length += ComputeDistance(bus->stops[i]->coordinates, bus->stops[i + 1]->coordinates);
        if (!bus->is_circular){
            real_route_length += GetDistance(bus->stops[i], bus->stops[i + 1]) + GetDistance(bus->stops[n], bus->stops[n - 1]);
            --n;
        }
        else {
            real_route_length += GetDistance(bus->stops[i], bus->stops[i + 1]);
        }
    }
    return {geo_route_length, real_route_length};
}

Stop* TransportCatalogue::GetStopPointer(std::string_view stop_name) const{
    return stop_names_to_stop_.at(stop_name);
}

const std::unordered_map<std::string_view, Bus*>& TransportCatalogue::GetAllBuses() const {
    return bus_names_to_bus_;
}

size_t TransportCatalogue::GetStopsNumber() const {
    return stops_.size();
}

    namespace detail {
size_t PairOfPointersHasher::operator()(const std::pair<Stop*, Stop*>& pair) const{
    return std::hash<Stop*>{}(pair.first) + 59 * std::hash<Stop*>{}(pair.second);
}
    }
}