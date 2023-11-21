#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "json_builder.h"

#include <unordered_map>

namespace router {

    using VertexId = size_t;
    using EdgeId = size_t;

    struct RouterSettings {
        double bus_velocity = 0;
        int bus_wait_time = 0.0;
    };

    struct Segment {
        transport_catalogue::Stop* from;
        transport_catalogue::Stop* to;
        const transport_catalogue::Bus* bus;
        double time = 0.0;
        int span_count = 0;
    };
   
    struct RouteSegments {
        std::vector<Segment> segments;
        double total_time = 0.0;
        double bus_wait_time = 0.0;
    };


class TransportRouter {
    public:
    
    TransportRouter(const transport_catalogue::TransportCatalogue& tc, RouterSettings settings) : tc_(tc), settings_(settings), graph_(BuildGraph()), router_(graph_)
    {
    }

    TransportRouter(std::unordered_map<std::string, VertexId>&& stop_to_vertexid,
                    std::unordered_map<VertexId, std::string>&& vertexid_to_stop,
                    std::unordered_map<EdgeId, std::string>&& edges_to_bus,
                    const transport_catalogue::TransportCatalogue& tc,
                    RouterSettings&& settings,
                    graph::DirectedWeightedGraph<double>&& graph,
                    graph::Router<double>::RoutesInternalData&& int_data) : stop_to_vertexid_(std::move(stop_to_vertexid))
                                                                          , vertexid_to_stop_(std::move(vertexid_to_stop))
                                                                          , edges_to_bus_(std::move(edges_to_bus))
                                                                          , tc_(tc)
                                                                          , settings_(std::move(settings))
                                                                          , graph_(std::move(graph))
                                                                          , router_(graph_, std::move(int_data))
    {
    }

    std::optional<RouteSegments> FindRoute(const std::string& from, const std::string& to) const;

    const std::unordered_map<std::string, VertexId>& GetStopToVertexId() const;
    const std::unordered_map<VertexId, std::string>& GetVertexIdToStop() const;
    const std::unordered_map<EdgeId, std::string>& GetEdgesToBus() const;
    const graph::DirectedWeightedGraph<double>& GetGraph() const;
    const graph::Router<double>& GetRouter() const;

    private:

    void FillGraph(const std::unordered_map<std::string_view, transport_catalogue::Bus*> buses, graph::DirectedWeightedGraph<double>& tmp);

    graph::DirectedWeightedGraph<double> BuildGraph();

    double CalculateDistanceBetweenStops(const std::vector<transport_catalogue::Stop*>& stops, size_t from, size_t to) const;
    
    double CalculateDistanceBetweenStopsReversed(const std::vector<transport_catalogue::Stop*>& stops, size_t from, size_t to) const;
    
    std::unordered_map<std::string /*Stop*/, VertexId> stop_to_vertexid_;
    std::unordered_map<VertexId, std::string /*Stop*/> vertexid_to_stop_;
    std::unordered_map<EdgeId, std::string /*Bus*/> edges_to_bus_;
    const transport_catalogue::TransportCatalogue& tc_;
    RouterSettings settings_;
    graph::DirectedWeightedGraph<double> graph_;
    graph::Router<double> router_;

};

} //namespace router