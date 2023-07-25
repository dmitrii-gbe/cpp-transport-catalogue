#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "transport_catalogue.h"
#include "geo.h"
#include "json.h"
#include "map_renderer.h"

namespace transport_catalogue {
    namespace request_handler{

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(TransportCatalogue& db, const renderer::MapRenderer& renderer);


    void FillDB(const json::Document& doc);
    void RespondToRequest(const json::Document& doc, std::ostream& out) const;
   

    
    // Возвращает информацию о маршруте (запрос Bus)
   // std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    //const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
   // svg::Document RenderMap() const;

private:

    void AddStops(const json::Array& queries_to_add);
    void AddBuses(const json::Array& queries_to_add);
    void SetDistances(const json::Array& queries_to_add);

    json::Node BusOutput(const std::string& name, int id) const;

    json::Node StopOutput(const std::string& name, int id) const;

    void RenderRoutesMap(std::ostream& out) const;
    
    json::Node MapOutPut(const int reques_id) const;
    //std::map<std::string_view, std::vector<geo::Coordinates>> GetRoutes() const;
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    TransportCatalogue& tc_;
    const renderer::MapRenderer& renderer_;
};
    }
}


