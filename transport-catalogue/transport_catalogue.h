#pragma once

#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <string_view>

namespace transport_catalogue {

struct Stop {
    Stop(std::string& name, double latitude, double longitude) : name(name), latitude(latitude), longitude(longitude)
	{
	}
    std::string name;
    double latitude = 0.0;
    double longitude = 0.0;
};  
    
struct Bus {
    Bus(std::string& name, std::vector<Stop*>& stops, bool is_circular) : name(name), stops(stops), is_circular(is_circular)
    {
    }
    std::string name;
    std::vector<Stop*> stops;
    bool is_circular = false;
};

namespace detail {
class PairOfPointersHasher {
    public:
    size_t operator()(const std::pair<Stop*, Stop*>& pair) const;
};
}

class TransportCatalogue {

    public:

    void AddBus(std::tuple<std::string, std::vector<std::string>, bool>&& bus);

    void AddStop(const Stop& stop);

    Bus* FindBus(const std::string& name) const;

    std::optional<const std::set<std::string_view>> GetBusesForStop(const std::string& stop) const;

    void AddDistances(const std::vector<std::pair<std::string, int>>& v, const std::string& name);

    int GetDistances(const std::pair<Stop*, Stop*> stops_pair) const;
    
    private:    
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Bus*> bus_names_to_bus_;
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, Stop*> stop_names_to_stop_;
        std::unordered_map<std::pair<Stop*, Stop*>, int, transport_catalogue::detail::PairOfPointersHasher> distances_;
};

}
