#include "json_reader.h"

namespace json_reader {

json::Document GetQuery(std::istream& input){
    json::Document queries = json::Load(input);
    return queries;
}

void GetColorSettings(renderer::RenderingSettings& result, const json::Dict& settings){

    if (settings.at("underlayer_color").IsArray()){
        json::Array ar = settings.at("underlayer_color").AsArray();
        if (ar.size() == 3){
            svg::Rgb rgb_color{static_cast<uint8_t>(ar[0].AsInt()), static_cast<uint8_t>(ar[1].AsInt()), static_cast<uint8_t>(ar[2].AsInt())};
            result.underlayer_color = rgb_color; 
        }
        if (ar.size() == 4) {
            svg::Rgba rgba_color{static_cast<uint8_t>(ar[0].AsInt()), static_cast<uint8_t>(ar[1].AsInt()), static_cast<uint8_t>(ar[2].AsInt()), ar[3].AsDouble()};
            result.underlayer_color = rgba_color; 
        }
    }
    if (settings.at("underlayer_color").IsString()) {
        result.underlayer_color = svg::Color(settings.at("underlayer_color").AsString());
    }

    json::Array ar = settings.at("color_palette").AsArray();
    for (const auto& item : ar){
        if (item.IsString()){
            result.color_palette.push_back(svg::Color(item.AsString()));
        }
        if (item.IsArray()){
            json::Array ar = item.AsArray();
            if (ar.size() == 3){
                svg::Rgb rgb_color{static_cast<uint8_t>(ar[0].AsInt()), static_cast<uint8_t>(ar[1].AsInt()), static_cast<uint8_t>(ar[2].AsInt())};
                result.color_palette.push_back(svg::Color(rgb_color));
            }
            if (ar.size() == 4) {
                svg::Rgba rgba_color{static_cast<uint8_t>(ar[0].AsInt()), static_cast<uint8_t>(ar[1].AsInt()), static_cast<uint8_t>(ar[2].AsInt()), ar[3].AsDouble()};
                result.color_palette.push_back(svg::Color(rgba_color));
            }
        }
    }    
}

renderer::RenderingSettings GetRenderingSettings(const json::Document& document){
    json::Dict settings = document.GetRoot().AsMap().at("render_settings").AsMap();
    renderer::RenderingSettings result;
    result.width = settings.at("width").AsDouble();
    result.height = settings.at("height").AsDouble();
    result.padding = settings.at("padding").AsDouble();
    result.line_width = settings.at("line_width").AsDouble();
    result.stop_radius = settings.at("stop_radius").AsDouble();
    result.bus_label_font_size = settings.at("bus_label_font_size").AsInt();
    result.underlayer_width = settings.at("underlayer_width").AsDouble();
    
    json::Array bus_label_offset_arr = settings.at("bus_label_offset").AsArray();
    renderer::LabelOffset bus_label_offset = {bus_label_offset_arr[0].AsDouble(), bus_label_offset_arr[1].AsDouble()};
    result.bus_label_offset = bus_label_offset;
    
    result.stop_label_font_size = settings.at("stop_label_font_size").AsInt();

    json::Array stop_label_offset_arr = settings.at("stop_label_offset").AsArray();
    renderer::LabelOffset stop_label_offset = {stop_label_offset_arr[0].AsDouble(), stop_label_offset_arr[1].AsDouble()};
    result.stop_label_offset = stop_label_offset;

    GetColorSettings(result, settings);

    return result;
}

router::RouterSettings GetRouterSettings(const json::Document& document){
    json::Dict settings = document.GetRoot().AsMap().at("routing_settings").AsMap();
    router::RouterSettings result;
    double velocity = settings.at("bus_velocity").AsDouble();
    double velocity_as_meter_per_min = velocity * VELOCITY_TO_METERS_PER_MIN;
    result.bus_velocity = velocity_as_meter_per_min;
    result.bus_wait_time = settings.at("bus_wait_time").AsInt();
    return result;
}

void AddStops(const json::Array& queries_to_add, transport_catalogue::TransportCatalogue& tc){
    for (const auto& item : queries_to_add){
        const auto dict = item.AsMap();
        if (dict.at("type").AsString() == "Stop"){
            geo::Coordinates coordinates = {dict.at("latitude").AsDouble(), dict.at("longitude").AsDouble()};
            std::string name = dict.at("name").AsString();
            transport_catalogue::Stop stop(name, coordinates);
            tc.AddStop(stop);
        }
    }
}

void AddBuses(const json::Array& queries_to_add, transport_catalogue::TransportCatalogue& tc){
    for (const auto& item : queries_to_add){
        const auto dict = item.AsMap();
        if (dict.at("type").AsString() == "Bus"){
            std::string name = dict.at("name").AsString();
            bool is_circular = dict.at("is_roundtrip").AsBool();
            std::vector<transport_catalogue::Stop*> stops;
            for (const auto& item : dict.at("stops").AsArray()){
                std::string s = item.AsString();
                stops.push_back(tc.GetStopPointer(s));
            }
            transport_catalogue::Bus bus(name, stops, is_circular);
            tc.AddBus(bus);
        }
    }
}
void SetDistances(const json::Array& queries_to_add, transport_catalogue::TransportCatalogue& tc){
    for (const auto& item : queries_to_add){
        const auto dict = item.AsMap();
        if (dict.at("type").AsString() == "Stop"){
            std::string name = dict.at("name").AsString();
            const auto dist_map = dict.at("road_distances").AsMap();
            for (const auto& stop : dist_map){
                tc.SetDistance(stop.first, name, stop.second.AsInt());
            }
        }
    }
}

void FillDB(const json::Document& doc, transport_catalogue::TransportCatalogue& tc){
    json::Array queries_to_add = doc.GetRoot().AsMap().at("base_requests").AsArray();
    AddStops(queries_to_add, tc);
    AddBuses(queries_to_add, tc);
    SetDistances(queries_to_add, tc);
}

void RenderRoutesMap(std::ostream& out, const renderer::MapRenderer& renderer, const transport_catalogue::TransportCatalogue& tc) {
   renderer.RenderRoutes(renderer.TranformCoordinates(tc.GetAllBuses())).Render(out);
}


json::Node GetMapOutput(const int request_id, const renderer::MapRenderer& renderer, const transport_catalogue::TransportCatalogue& tc) {
    std::ostringstream ss;
    RenderRoutesMap(ss, renderer, tc);
    std::string map = ss.str();
    return json::Builder{}.StartDict().Key("map").Value(map).Key("request_id").Value(request_id).EndDict().Build();
}

json::Node GetBusOutput(const std::string& name, int id, const transport_catalogue::TransportCatalogue& tc) {
    json::Dict result;
    result["request_id"] = json::Builder{}.Value(id).Build();
    const transport_catalogue::Bus* bus = tc.FindBus(name);
    if (bus == nullptr){
        result["error_message"] = json::Builder{}.Value("not found").Build();
    }
    else {
        auto geo_and_real_route_lengths = tc.CalculateRouteLength(bus);
        double geo_route_length = geo_and_real_route_lengths.first;
        double real_route_length = geo_and_real_route_lengths.second;
        std::unordered_set<transport_catalogue::Stop*> unique_elements(bus->stops.begin(), bus->stops.end());
        int stop_count = (bus->is_circular == true ? bus->stops.size() : bus->stops.size() * 2 - 1);
        int u_stop_count = unique_elements.size();
        double curvature = ((bus->is_circular == true ? real_route_length / geo_route_length : real_route_length / geo_route_length / 2.0));
        result["curvature"] = json::Builder{}.Value(curvature).Build();
        result["route_length"] = json::Builder{}.Value(real_route_length).Build();
        result["stop_count"] = json::Builder{}.Value(stop_count).Build();
        result["unique_stop_count"] = json::Builder{}.Value(u_stop_count).Build();
    }
    return json::Builder{}.Value(result).Build();
}

json::Node GetStopOutput(const std::string& name, int id, const transport_catalogue::TransportCatalogue& tc) {
    json::Dict result;
    result["request_id"] = json::Builder{}.Value(id).Build();
    std::vector<json::Node> array_of_buses;
    auto buses = tc.GetBusesForStop(name);
    if (!buses.has_value()){
        result["error_message"] = json::Builder{}.Value("not found").Build();
    }
    else {
        for (const auto& bus : buses.value()){
            json::Node bus_name{static_cast<std::string>(bus)};
            array_of_buses.push_back(bus_name);
        }
        result["buses"] = json::Builder{}.Value(array_of_buses).Build();
    }
    return json::Builder{}.Value(result).Build();
}

json::Node GetRouteOutput(int id, std::string_view from, std::string_view to, const router::TransportRouter& router, const transport_catalogue::TransportCatalogue& tc_) {
    json::Dict result;
    result["request_id"] = json::Builder{}.Value(id).Build();
    auto route = router.FindRoute(tc_.GetStopPointer(from), tc_.GetStopPointer(to)); 
    if (!route.first.has_value()){
        result["error_message"] = json::Builder{}.Value("not found").Build();
    }
    else {
        result["total_time"] = json::Builder{}.Value(route.second).Build();
        result["items"] = *route.first;
    }
    return json::Builder{}.Value(result).Build();
}

void RespondToRequest(const json::Document& doc, std::ostream& out, const renderer::MapRenderer& renderer_, const transport_catalogue::TransportCatalogue& tc_, const router::TransportRouter& router){
    json::Array queries_to_respond = doc.GetRoot().AsMap().at("stat_requests").AsArray();
    std::vector<json::Node> result;
    for (const auto& item : queries_to_respond){
        const auto resp_map = item.AsMap();
        if (resp_map.at("type").AsString() == "Stop"){
            result.push_back(json_reader::GetStopOutput(resp_map.at("name").AsString(), resp_map.at("id").AsInt(), tc_));
        }
        if (resp_map.at("type").AsString() == "Bus"){
            result.push_back(json_reader::GetBusOutput(resp_map.at("name").AsString(), resp_map.at("id").AsInt(), tc_));
        }
        if (resp_map.at("type").AsString() == "Map"){
            result.push_back(json_reader::GetMapOutput(resp_map.at("id").AsInt(), renderer_, tc_));
        }
        if (resp_map.at("type").AsString() == "Route"){
            result.push_back(json_reader::GetRouteOutput(resp_map.at("id").AsInt(), resp_map.at("from").AsString(), resp_map.at("to").AsString(),  router, tc_));
        }
    }
    json::Print(json::Document(json::Builder{}.Value(result).Build()), out);
}

}
