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


class TransportRouter {
    public:
    
    TransportRouter(const transport_catalogue::TransportCatalogue& tc, RouterSettings settings) : tc_(tc), settings_(settings), graph_(BuildGraph()), router_(graph_)
    {
    } 

    std::optional<graph::Router<double>::RouteInfo> FindRoute(transport_catalogue::Stop* from, transport_catalogue::Stop* to) const;

    VertexId GetVertexIdByStop(transport_catalogue::Stop* stop_ptr) const;
    transport_catalogue::Stop* GetStopByVertexId(VertexId id) const;
    transport_catalogue::Bus* GetBusByEdgeId(EdgeId id) const;
    const graph::Edge<double>& GetEdgeByEdgeId(EdgeId id) const;
    int GetBusWaitTime() const;

    private:

    void FillStopDictionaries(const std::unordered_map<std::string_view, transport_catalogue::Bus*> buses);

    void FillGraph(const std::unordered_map<std::string_view, transport_catalogue::Bus*> buses, graph::DirectedWeightedGraph<double>& tmp);

    graph::DirectedWeightedGraph<double> BuildGraph();

    double CalculateDistanceBetweenStops(const std::vector<transport_catalogue::Stop*>& stops, size_t from, size_t to) const;
    
    double CalculateDistanceBetweenStopsReversed(const std::vector<transport_catalogue::Stop*>& stops, size_t from, size_t to) const;
    
    std::unordered_map<transport_catalogue::Stop*, VertexId> stop_to_vertexid_;
    std::unordered_map<VertexId, transport_catalogue::Stop*> vertexid_to_stop_;
    std::unordered_map<EdgeId, transport_catalogue::Bus*> edges_to_bus_;
    const transport_catalogue::TransportCatalogue& tc_;
    RouterSettings settings_;
    graph::DirectedWeightedGraph<double> graph_;
    graph::Router<double> router_;

};

} //namespace router