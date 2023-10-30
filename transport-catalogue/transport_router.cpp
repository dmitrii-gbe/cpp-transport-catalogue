#include "transport_router.h"

namespace router {   
   
    std::optional<RouteSegments> TransportRouter::FindRoute(transport_catalogue::Stop* from, transport_catalogue::Stop* to) const {   
        if (stop_to_vertexid_.count(from) == 0 || stop_to_vertexid_.count(to) == 0){
            return std::nullopt;
        }
        std::optional<graph::Router<double>::RouteInfo> route = router_.BuildRoute(stop_to_vertexid_.at(from), stop_to_vertexid_.at(to)); //возвращает Weight и вектор граней
        if (!route.has_value()){
            return std::nullopt;
        }
        else {
            RouteSegments result;
            result.total_time = route.value().weight;
            result.bus_wait_time = settings_.bus_wait_time;
            for (const auto& edge_id : route.value().edges){
                auto edge = graph_.GetEdge(edge_id);
                result.segments.push_back({vertexid_to_stop_.at(edge.from),
                                            vertexid_to_stop_.at(edge.to),
                                            edges_to_bus_.at(edge_id),
                                            edge.weight,
                                            edge.span_count});
            }
        return result;
        }
    }

    void TransportRouter::FillGraph(const std::unordered_map<std::string_view, transport_catalogue::Bus*> buses, graph::DirectedWeightedGraph<double>& tmp){
        size_t i = 0;
        for (const auto& bus : buses){
            const std::vector<transport_catalogue::Stop*>& stops = bus.second->stops;
            for (const auto& stop : stops){
                if (stop_to_vertexid_.count(stop) == 0){
                    vertexid_to_stop_[i] = stop;
                    stop_to_vertexid_[stop] = i++;
                }
            }
        }

         for (const auto& bus : buses){
            const std::vector<transport_catalogue::Stop*>& stops = bus.second->stops;
            if (!bus.second->is_circular){
                for (size_t i = 0; i < stops.size(); ++i){
                    for (size_t j = i + 1; j < stops.size(); ++j){
                        int span_count = j - i;
                        double time = (CalculateDistanceBetweenStops(stops, i, j) / settings_.bus_velocity) + settings_.bus_wait_time;
                        EdgeId id = tmp.AddEdge({stop_to_vertexid_.at(stops[i]), stop_to_vertexid_.at(stops[j]), time, span_count});
                        edges_to_bus_[id] = bus.second;
                        double time_reversed = (CalculateDistanceBetweenStopsReversed(stops, j, i) / settings_.bus_velocity) + settings_.bus_wait_time;
                        EdgeId id_reversed = tmp.AddEdge({stop_to_vertexid_.at(stops[j]), stop_to_vertexid_.at(stops[i]), time_reversed, span_count});
                        edges_to_bus_[id_reversed] = bus.second;
                    }
                }            
            }
            else {
                for (size_t i = 0; i < stops.size(); ++i){
                    for (size_t j = i + 1; j < stops.size(); ++j){
                        int span_count = j - i;
                        double time = (CalculateDistanceBetweenStops(stops, i, j) / settings_.bus_velocity) + settings_.bus_wait_time;
                        EdgeId id = tmp.AddEdge({stop_to_vertexid_.at(stops[i]), stop_to_vertexid_.at(stops[j]), time, span_count});
                        edges_to_bus_[id] = bus.second;
                    }
                }
            }
        }
    }
    
   
    graph::DirectedWeightedGraph<double> TransportRouter::BuildGraph(){
        graph::DirectedWeightedGraph<double> tmp(tc_.GetStopsNumber());
        auto buses = tc_.GetAllBuses();
        FillGraph(buses, tmp);
        return tmp;
    }     

    double TransportRouter::CalculateDistanceBetweenStops(const std::vector<transport_catalogue::Stop*>& stops, size_t from, size_t to) const {
        double distance = 0.0;
        if (to - from > 1){
            size_t i = from + 1;
            while (i <= to){
                distance += tc_.GetDistance(stops[from], stops[i]);
                ++from;
                ++i;
            }
        }
        else {
            distance = tc_.GetDistance(stops[from], stops[to]);
        }

        return distance;
    }

    
    double TransportRouter::CalculateDistanceBetweenStopsReversed(const std::vector<transport_catalogue::Stop*>& stops, size_t from, size_t to) const {
        double distance = 0.0;
        if (from - to > 1){
            size_t i = from - 1;
            while (i >= to){
                distance += tc_.GetDistance(stops[from], stops[i]);
                --from;
                if (i > 0){
                    --i;
                }
                else {
                    return distance;
                }                
            }
        }
        else {
            distance += tc_.GetDistance(stops[from], stops[to]);
        }
        return distance;
    }


    }