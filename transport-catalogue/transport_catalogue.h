#pragma once

#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <string_view>

#include "geo.h"

namespace transport_catalogue {

struct Stop {
    Stop(std::string& name, const geo::Coordinates& coordinates) : name(name), coordinates(coordinates)
	{
	}
    std::string name;
    geo::Coordinates coordinates;
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

    void AddBus(const Bus& bus);

    void AddStop(const Stop& stop);

    const Bus* FindBus(const std::string& name) const;
    /*
    Вариант с константным указателем на Bus выдаёт ошибку компиляции в тренажёре "error: type qualifiers ignored on function return type [-Werror=ignored-qualifiers]"
    Проблема, по-видимому, во флаге -pedantic, хотя g++ на Windows с параметрами -Wall -Werror -pedantic компилирует без проблем.
    Тренажёр позволяет сделать константным только само значение
    */

    std::optional<const std::set<std::string_view>> GetBusesForStop(const std::string& stop) const;

    void SetDistance(const std::string& stop_name_to, const std::string& stop_name_from, const int distance);

    double GetDistance(Stop* const from, Stop* const to) const;

    std::pair<double, double> CalculateRouteLength(const Bus* bus) const;

    //Stop* GetStopPointer(const std::string& stop_name) const;

    Stop* GetStopPointer(std::string_view stop_name) const;

    const std::unordered_map<std::string_view, Bus*>& GetAllBuses() const;

    size_t GetStopsNumber() const;
    
    private:    
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Bus*> bus_names_to_bus_;
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, Stop*> stop_names_to_stop_;
        std::unordered_map<std::pair<Stop*, Stop*>, int, transport_catalogue::detail::PairOfPointersHasher> distances_;
};

}
