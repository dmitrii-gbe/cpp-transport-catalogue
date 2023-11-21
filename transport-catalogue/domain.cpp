#include "domain.h"


#include <iomanip>

#include <algorithm>


namespace transport_catalogue {
    namespace detail {

const std::string GetFlag(const std::string& s){
    size_t position = s.find_first_of(" ", 0);
    return s.substr(0, position);
}
    }

    namespace input_reader {
std::vector<std::string> ReadQuery(){
    int query_count;
    std::vector<std::string> queries; 
    std::cin >> query_count;
    std::string tmp;
    std::getline(std::cin, tmp);
    while (query_count > 0){
        std::string query;
        std::getline(std::cin, query);
        queries.push_back(query);
        --query_count;
    }
    return queries;
}

std::tuple<Stop, size_t> ParseStop(const std::string& s, const size_t name_position){
    size_t position = s.find_first_of(":", name_position);
    std::string name = s.substr(name_position, position - name_position);
	size_t position_lat = s.find_first_not_of(" ", ++position);
	size_t position_comma = s.find_first_of(",", ++position);
    double latitude = std::stod(s.substr(position_lat, position_comma - position_lat));
	size_t position_lon = s.find_first_not_of(" ", ++position_comma);
    size_t position_comma_lon = s.find_first_of(",", position_lon);
	double longitude = std::stod(s.substr(position_lon, position_comma_lon - position_lon));
    size_t position_distance = s.find_first_not_of(", ", position_comma_lon);
    const geo::Coordinates coordinates = {latitude, longitude};
    Stop stop = {name, coordinates};
	return {stop, position_distance};
}

const std::vector<std::pair<std::string, int>> ParseDistances(const std::string& s, size_t position){
    std::vector<std::string> v;
    while (position != std::string::npos){
        size_t position_comma = s.find_first_of(",", position);
        v.push_back(s.substr(position, position_comma - position));
        position = s.find_first_not_of(", ", position_comma);
    }
    std::vector<std::pair<std::string, int>> v_distances;
        for (const auto& entry : v){
            size_t m_position = entry.find_first_of("m");
            int distance = std::stoi(entry.substr(0, m_position));
            size_t to_position = entry.find(" to ");
            size_t name_position = entry.find_first_not_of(" ", to_position + 3);
            std::string entry_name = entry.substr(name_position);
            v_distances.push_back({entry_name, distance});
        }
    return v_distances;
}

const Bus ParseBus(std::string&& s, const TransportCatalogue& tc){
    size_t position = s.find_first_of(":");
    std::string name = s.substr(0, position);
    bool is_circular = false;
    size_t delimiter_position = s.find_first_of(">-", position);
    if (s[delimiter_position] == '>'){
        is_circular = true;
    }
    size_t point = 0;
    std::vector<Stop*> stops;
    while (position != std::string::npos){
        point = s.find_first_not_of(">- :", position);
        size_t point_ws = s.find_first_of(">-", point);
        stops.push_back(tc.GetStopPointer(s.substr(point, point_ws - point - 1)));
        position = point_ws;
    }
    return {name, stops, is_circular};
}

void ParseQueryToAdd(TransportCatalogue& tc, std::vector<std::string>&& query){
    std::sort(query.begin(), query.end(), [](const std::string& lhs, const std::string& rhs){ return rhs < lhs; });
    std::vector<std::pair<std::string, std::pair<size_t, std::string>>> stop_strings;
    for (const auto& s : query){
        size_t position = s.find_first_of(" ", 0);
        const auto flag = transport_catalogue::detail::GetFlag(s);
        if (flag == "Stop"){
            const auto result = ParseStop(s, ++position);
            const Stop stop_to_add = std::get<0>(result);
            tc.AddStop(stop_to_add);
            std::pair<size_t, std::string> pair = { std::get<1>(result), stop_to_add.name };
            stop_strings.push_back({std::move(s), pair});
        }
        if (flag == "Bus"){
            tc.AddBus(ParseBus(s.substr(++position), tc));
        }
    }
    for (const auto& entry : stop_strings){
        auto pair = entry.second; //a pair of first character position of distance substring and stop name "from"
        auto vector_of_distances = ParseDistances(entry.first, pair.first); //accept the orignal string and a position of the first character of distance substring; returns a vector of pairs containing a stop name "to" and a distance to this stop 
        for (const auto& dist_to_stop : vector_of_distances){
            tc.SetDistance(dist_to_stop.first, pair.second, dist_to_stop.second);  //accept stop name "to", stop name "from", and distance between them
        }              
    }
}
    }
}


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
    for (const auto& s : query){
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
