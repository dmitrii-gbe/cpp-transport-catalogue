#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddBus(std::tuple<std::string, std::vector<std::string>, bool>&& bus){
    std::vector<Stop*> stops;
    for (const auto& stop : std::get<1>(bus)){
        stops.push_back(stop_names_to_stop_.at(stop));
    }
    buses_.push_back({std::get<0>(bus), stops, std::get<2>(bus)});
    bus_names_to_bus_.emplace(prev(buses_.end())->name, &(*prev(buses_.end())));
}

void TransportCatalogue::AddStop(const Stop& stop){
    stops_.push_back(stop);
    stop_names_to_stop_.emplace(prev(stops_.end())->name, &(*prev(stops_.end())));
}

Bus* TransportCatalogue::FindBus(const std::string& name) const {
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

void TransportCatalogue::AddDistances(const std::vector<std::pair<std::string, int>>& v, const std::string& name){
    for (const auto& entry : v){
        const std::pair<Stop*, Stop*> ptr_pair = {stop_names_to_stop_.at(name), stop_names_to_stop_.at(entry.first)};
        distances_.emplace(ptr_pair, entry.second);
    }
}

int TransportCatalogue::GetDistances(const std::pair<Stop*, Stop*> stops_pair) const{
    if (distances_.count(stops_pair) != 0){
        return distances_.at(stops_pair);
    }
    else {
        const std::pair rev_pair = {stops_pair.second, stops_pair.first};
        return distances_.at(rev_pair);
    }
}

    namespace detail {
size_t PairOfPointersHasher::operator()(const std::pair<Stop*, Stop*>& pair) const{
    return std::hash<Stop*>{}(pair.first) + 59 * std::hash<Stop*>{}(pair.second);
}
    }
}