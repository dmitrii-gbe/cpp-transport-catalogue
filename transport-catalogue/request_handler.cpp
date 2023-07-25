#include "request_handler.h"
#include <iomanip>
#include <algorithm>

namespace transport_catalogue {
    namespace request_handler {

RequestHandler::RequestHandler(TransportCatalogue& db, const renderer::MapRenderer& renderer) : tc_(db), renderer_(renderer) 
{
}

void RequestHandler::FillDB(const json::Document& doc){
    json::Array queries_to_add = doc.GetRoot().AsMap().at("base_requests").AsArray();
    AddStops(queries_to_add);
    AddBuses(queries_to_add);
    SetDistances(queries_to_add);
}

void RequestHandler::AddStops(const json::Array& queries_to_add){
    for (const auto& item : queries_to_add){
        const auto dict = item.AsMap();
        if (dict.at("type").AsString() == "Stop"){
            geo::Coordinates coordinates = {dict.at("latitude").AsDouble(), dict.at("longitude").AsDouble()};
            std::string name = dict.at("name").AsString();
            transport_catalogue::Stop stop(name, coordinates);
            tc_.AddStop(stop);
        }
    }
}

void RequestHandler::AddBuses(const json::Array& queries_to_add){
    for (const auto& item : queries_to_add){
        const auto dict = item.AsMap();
        if (dict.at("type").AsString() == "Bus"){
            std::string name = dict.at("name").AsString();
            bool is_circular = dict.at("is_roundtrip").AsBool();
            std::vector<transport_catalogue::Stop*> stops;
            for (const auto& item : dict.at("stops").AsArray()){
                std::string s = item.AsString();
                stops.push_back(tc_.GetStopPointer(s));
            }
            transport_catalogue::Bus bus(name, stops, is_circular);
            tc_.AddBus(bus);
        }
    }
}
void RequestHandler::SetDistances(const json::Array& queries_to_add){

    for (const auto& item : queries_to_add){
        const auto dict = item.AsMap();
        if (dict.at("type").AsString() == "Stop"){
            std::string name = dict.at("name").AsString();
            const auto dist_map = dict.at("road_distances").AsMap();
            for (const auto& stop : dist_map){
                tc_.SetDistance(stop.first, name, stop.second.AsInt());
            }
        }
    }
}

void RequestHandler::RespondToRequest(const json::Document& doc, std::ostream& out) const{
    json::Array queries_to_respond = doc.GetRoot().AsMap().at("stat_requests").AsArray();
    std::vector<json::Node> result;
    for (const auto& item : queries_to_respond){
        const auto resp_map = item.AsMap();
        if (resp_map.at("type").AsString() == "Stop"){
            result.push_back(StopOutput(resp_map.at("name").AsString(), resp_map.at("id").AsInt()));
        }
        if (resp_map.at("type").AsString() == "Bus"){
            result.push_back(BusOutput(resp_map.at("name").AsString(), resp_map.at("id").AsInt()));
        }
        if (resp_map.at("type").AsString() == "Map"){
            result.push_back(MapOutPut(resp_map.at("id").AsInt()));
        }
    }
    json::Document d(result);
    json::Print(d, out);
}

json::Node RequestHandler::MapOutPut(const int request_id) const {
    std::ostringstream ss;
    RenderRoutesMap(ss);
    std::string map = ss.str();
    json::Dict dict;
    dict["map"] = json::Node(map);
    dict["request_id"] = json::Node{request_id};
    json::Node result{dict};
    return result;
}

json::Node RequestHandler::BusOutput(const std::string& name, int id) const {
    json::Dict request_id;
    json::Dict result;
    result["request_id"] = json::Node{id};
    const Bus* bus = tc_.FindBus(name);
    if (bus == nullptr){
        std::string s = "not found";
        json::Node err_string(s);
        result["error_message"] = err_string;
    }
    else {
        auto geo_and_real_route_lengths = tc_.CalculateRouteLength(bus);
        double geo_route_length = geo_and_real_route_lengths.first;
        double real_route_length = geo_and_real_route_lengths.second;
        std::unordered_set<Stop*> unique_elements(bus->stops.begin(), bus->stops.end());
        int stop_count = (bus->is_circular == true ? bus->stops.size() : bus->stops.size() * 2 - 1);
        int u_stop_count = unique_elements.size();
        double curvature = ((bus->is_circular == true ? real_route_length / geo_route_length : real_route_length / geo_route_length / 2.0));
        result["curvature"] = json::Node{curvature};
        result["route_length"] = json::Node{real_route_length};
        result["stop_count"] = json::Node{stop_count};
        result["unique_stop_count"] = json::Node{u_stop_count};
    }
    return json::Node{result};
}

json::Node RequestHandler::StopOutput(const std::string& name, int id) const{
    json::Dict request_id;
    json::Dict result;
    result["request_id"] = json::Node{id};
    std::vector<json::Node> array_of_buses;
    auto buses = tc_.GetBusesForStop(name);
    if (!buses.has_value()){
        std::string s = "not found";
        json::Node err_string(s);
        result["error_message"] = err_string;
    }
    else {
        for (const auto& bus : buses.value()){
            json::Node bus_name(static_cast<std::string>(bus));
            array_of_buses.push_back(bus_name);
        }
        json::Node array(array_of_buses);
        result["buses"] = array;
    }
    return json::Node{result};
}


void RequestHandler::RenderRoutesMap(std::ostream& out) const {
   renderer_.RenderRoutes(renderer_.TranformCoordinates(tc_.GetAllBuses())).Render(out);
}

    }
}






/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */