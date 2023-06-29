#include <algorithm>
#include "input_reader.h"

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
	return {{name, latitude, longitude}, position_distance};
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

std::tuple<std::string, std::vector<std::string>, bool> ParseBus(std::string&& s){
    size_t position = s.find_first_of(":");
    std::string name = s.substr(0, position);
    bool is_circular = false;
    size_t delimiter_position = s.find_first_of(">-", position);
    if (s[delimiter_position] == '>'){
        is_circular = true;
    }
    size_t point = 0;
    std::vector<std::string> stops;
    while (position != std::string::npos){
        point = s.find_first_not_of(">- :", position);
        size_t point_ws = s.find_first_of(">-", point);
        stops.push_back(s.substr(point, point_ws - point - 1));
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
            tc.AddBus(ParseBus(s.substr(++position)));
        }
    }
    for (const auto& entry : stop_strings){
        auto pair = entry.second;
        tc.AddDistances(ParseDistances(entry.first, pair.first), pair.second);        
    }
}
    }
}
