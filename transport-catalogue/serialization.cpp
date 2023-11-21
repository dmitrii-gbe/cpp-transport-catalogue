#include "serialization.h"
#include "transport_catalogue.pb.h"
#include "transport_catalogue.h"
#include "geo.h"
#include <fstream>
#include <iostream>
#include <istream>
#include <optional>
#include <ostream>
#include "json_reader.h"
#include "json.h"
#include <string>


namespace myproto {

    template <typename T, typename G>
    google::protobuf::Map<T, G> CreateProtoMap (const std::unordered_map<T, G>& map){
        google::protobuf::Map<T, G> proto_map;
        for (const auto& [key, value] : map){
            proto_map[key] = value;
        }
        return proto_map;
    }

    google::protobuf::RepeatedPtrField<proto::Edge> CreateProtoEdges(const std::vector<graph::Edge<double>>& edges){
        google::protobuf::RepeatedPtrField<proto::Edge> result;
        for (const auto& edge : edges){
            proto::Edge* proto_edge = result.Add();
            proto_edge->set_from(edge.from);
            proto_edge->set_to(edge.to);
            proto_edge->set_weight(edge.weight);
            proto_edge->set_span_count(edge.span_count);
        }
        return result;
    }

    router::RouterSettings SetRouterSettings(const proto::TransportCatalogue& catalogue){
        router::RouterSettings result;
        result.bus_velocity = catalogue.bus_velocity();
        result.bus_wait_time = catalogue.bus_wait_time();
        return result;
    }

    proto::RGB CreateProtoRGB(const json::Array& array){
        proto::RGB rgb;
        rgb.set_r(array[0].AsInt());
        rgb.set_g(array[1].AsInt());
        rgb.set_b(array[2].AsInt());
        return rgb;
    }

    proto::RGBA CreateProtoRGBA(const json::Array& array){
        proto::RGBA rgba;
        rgba.set_r(array[0].AsInt());
        rgba.set_g(array[1].AsInt());
        rgba.set_b(array[2].AsInt());
        rgba.set_opacity(array[3].AsDouble());
        return rgba;
    }

    proto::TransportCatalogue SerializeTransportCatalogue(const transport_catalogue::TransportCatalogue& tc, const json::Document& queries){
        proto::TransportCatalogue catalogue;
        for (const auto& bus : tc.GetAllBuses()){
            proto::Bus* proto_bus = catalogue.add_buses_();
            proto_bus->set_name(std::string(bus.first));
            proto_bus->set_is_roundtrip(bus.second->is_circular);
            for (const auto& stop_ptr : bus.second->stops){
                proto::Stop* proto_stop = proto_bus->add_stops();
                proto_stop->set_name(stop_ptr->name);
                proto_stop->set_latitude(stop_ptr->coordinates.lat);
                proto_stop->set_longitude(stop_ptr->coordinates.lng);
            }
        }
        for (const auto& stop : queries.GetRoot().AsMap().at("base_requests").AsArray()){
            if (stop.AsMap().at("type").AsString() == "Stop"){
                auto s = catalogue.add_stops_();
                s->set_latitude(stop.AsMap().at("latitude").AsDouble());
                s->set_longitude(stop.AsMap().at("longitude").AsDouble());
                s->set_name(stop.AsMap().at("name").AsString());
            }
        }
        
        for (const auto& stop : queries.GetRoot().AsMap().at("base_requests").AsArray()){
            json::Dict base_request = stop.AsMap();
            if (base_request.at("type").AsString() == "Stop"){
                for (const auto& dist : base_request.at("road_distances").AsMap()){
                    proto::Dist* d = catalogue.add_distanses();
                    d->set_departure(stop.AsMap().at("name").AsString());
                    d->set_destination(dist.first);
                    d->set_distance(dist.second.AsDouble());
                }
            }
        }
        return catalogue;
    }

    proto::RenderSettings SerializeoRenderingSettings(const json::Dict& render_settings){
        proto::RenderSettings render;
        render.set_width(render_settings.at("width").AsDouble());
        render.set_height(render_settings.at("height").AsDouble());
        render.set_padding(render_settings.at("padding").AsDouble());
        render.set_line_width(render_settings.at("line_width").AsDouble());
        render.set_stop_radius(render_settings.at("stop_radius").AsDouble());
        render.set_underlayer_width(render_settings.at("underlayer_width").AsDouble());
        render.set_bus_label_font_size(render_settings.at("bus_label_font_size").AsInt());
        render.set_stop_label_font_size(render_settings.at("stop_label_font_size").AsInt());
        for (const auto& bus_offset : render_settings.at("bus_label_offset").AsArray()){
            render.add_bus_label_offset(bus_offset.AsDouble());
        }        
        for (const auto& stop_offset : render_settings.at("stop_label_offset").AsArray()){
            render.add_stop_label_offset(stop_offset.AsDouble());
        }
        proto::Color_palette underlayer_color;
        if (render_settings.at("underlayer_color").IsArray()){
            json::Array ar = render_settings.at("underlayer_color").AsArray();
            if (ar.size() == 3){
                *underlayer_color.mutable_rgb() = CreateProtoRGB(ar);
            }
            if (ar.size() == 4){
                *underlayer_color.mutable_rgba() = CreateProtoRGBA(ar);
            }
        }
        else {
            std::string color_name = render_settings.at("underlayer_color").AsString();
            underlayer_color.set_color_name(color_name);
        }
        for (const auto& color : render_settings.at("color_palette").AsArray()){

            proto::Color_palette p;
            if (color.IsString()){
                p.set_color_name(color.AsString());
            }
            else {
                json::Array ar = color.AsArray();
                if (ar.size() == 3){
                    *p.mutable_rgb() = CreateProtoRGB(ar);
                }
                if (ar.size() == 4){
                    *p.mutable_rgba() = CreateProtoRGBA(ar);
                }

            }
            *render.add_color_palette() = p;
        }

        *render.mutable_underlayer_color() = underlayer_color;
        return render;
    }

    void SetProtoRouter(const router::TransportRouter& router, proto::TransportCatalogue& catalogue){
        proto::Router proto_router;
        auto stop_to_vertex_id = proto_router.mutable_stop_to_vertexid();
        auto vertexid_to_stop = proto_router.mutable_vertexid_to_stop();
        auto edge_to_bus = proto_router.mutable_edge_to_bus();
        *stop_to_vertex_id = CreateProtoMap(router.GetStopToVertexId());
        *vertexid_to_stop = CreateProtoMap(router.GetVertexIdToStop());
        *edge_to_bus = CreateProtoMap(router.GetEdgesToBus());

        graph::DirectedWeightedGraph<double> router_graph = router.GetGraph();
        auto edges = router_graph.GetEdges();
        auto incidence_lists = router_graph.GetIncidentLists();

        auto proto_graph = proto_router.mutable_graph();

        auto proto_edges = proto_graph->mutable_edges();
        *proto_edges = CreateProtoEdges(edges);
        
        auto proto_router_ptr = catalogue.mutable_router();
        *proto_router_ptr = proto_router;

        auto int_data = router.GetRouter().GetRoutesInternalData();

        for (const auto& item_top : int_data){
            proto::RoutesInternalData* proto_int_data_item = catalogue.add_routes_internal_data();
            for (const auto& item_bottom : item_top){
                auto proto_int_data_ptr = proto_int_data_item->add_routes_internal_data_first(); 
                auto proto_int_data_ptr_ptr = proto_int_data_ptr->mutable_optional_route_internal_data();
                if (item_bottom.has_value()){                    
                    proto_int_data_ptr_ptr->set_weight(item_bottom.value().weight);
                    if (item_bottom.value().prev_edge.has_value()){
                        proto_int_data_ptr_ptr->set_prev_edge(item_bottom.value().prev_edge.value());
                    }
                    else {
                        proto_int_data_ptr_ptr->clear_prev_edge();
                    }
                }
                else {
                    proto_int_data_ptr_ptr->Clear();
                }
            }
        }
}

    void MakeBase(json::Document queries){
        
        transport_catalogue::TransportCatalogue tc;
        json_reader::FillDB(queries, tc);
        router::RouterSettings router_settings = json_reader::GetRouterSettings(queries);
        router::TransportRouter router(tc, router_settings);
        
        proto::TransportCatalogue catalogue = SerializeTransportCatalogue(tc, queries);

        auto render_settings = queries.GetRoot().AsMap().at("render_settings").AsMap();

        *catalogue.mutable_render_settings() = SerializeoRenderingSettings(render_settings);

        catalogue.set_bus_velocity(queries.GetRoot().AsMap().at("routing_settings").AsMap().at("bus_velocity").AsDouble() * json_reader::VELOCITY_TO_METERS_PER_MIN);
        catalogue.set_bus_wait_time(queries.GetRoot().AsMap().at("routing_settings").AsMap().at("bus_wait_time").AsDouble());

        SetProtoRouter(router, catalogue);        
        
        std::ofstream out(queries.GetRoot().AsMap().at("serialization_settings").AsMap().at("file").AsString(), std::ios_base::binary);
        catalogue.SerializeToOstream(&out);
        out.close();
    }

    router::TransportRouter DeserializeTransportRouter(const proto::TransportCatalogue& catalogue, const transport_catalogue::TransportCatalogue& tc){
        proto::Router proto_router = catalogue.router();
        auto proto_stop_to_vertex_id = proto_router.stop_to_vertexid();
        auto proto_vertex_id_to_stop = proto_router.vertexid_to_stop();
        auto proto_edge_to_bus = proto_router.edge_to_bus();
        auto proto_graph = proto_router.graph();
        std::unordered_map<std::string, router::VertexId> stop_to_vertex_id(proto_stop_to_vertex_id.begin(), proto_stop_to_vertex_id.end());
        std::unordered_map<router::VertexId, std::string> vertex_id_to_stop(proto_vertex_id_to_stop.begin(), proto_vertex_id_to_stop.end());
        std::unordered_map<router::EdgeId, std::string> edge_to_bus(proto_edge_to_bus.begin(), proto_edge_to_bus.end());
        router::RouterSettings settings = SetRouterSettings(catalogue);
        std::vector<graph::Edge<double>> edges;
        edges.reserve(proto_graph.edges_size());
        for (const auto& proto_edge : proto_graph.edges()){
            graph::Edge<double> edge;
            edge.from = proto_edge.from();
            edge.to = proto_edge.to();
            edge.weight = proto_edge.weight();
            edge.span_count = proto_edge.span_count();
            edges.push_back(edge);
        }

        auto proto_routes_int_data = catalogue.routes_internal_data();

        graph::Router<double>::RoutesInternalData internal_data;
        for (const auto& item_top : proto_routes_int_data){
            std::vector<std::optional<graph::Router<double>::RouteInternalData>> v;
            v.reserve(item_top.routes_internal_data_first_size());
            for (const auto& item_bottom : item_top.routes_internal_data_first()){
                if (item_bottom.optional_route_internal_data__case() == 1){
                    if (item_bottom.optional_route_internal_data().prev_edge__case() == 2){
                        v.push_back(std::optional<graph::Router<double>::RouteInternalData>({item_bottom.optional_route_internal_data().weight(), item_bottom.optional_route_internal_data().prev_edge()}));
                    }
                    else {
                        v.push_back(std::optional<graph::Router<double>::RouteInternalData>({item_bottom.optional_route_internal_data().weight(), std::nullopt}));
                    }
                }
                else {
                    v.push_back(std::nullopt);
                }
            }
            internal_data.push_back(std::move(v));
        }

        graph::DirectedWeightedGraph<double> graph(edges);

        return router::TransportRouter(std::move(stop_to_vertex_id)
                                     , std::move(vertex_id_to_stop)
                                     , std::move(edge_to_bus)
                                     , tc
                                     , std::move(settings)
                                     , std::move(graph)
                                     , std::move(internal_data));
    }

    void SetTransportCatalogue(const proto::TransportCatalogue& catalogue, transport_catalogue::TransportCatalogue& tc){
        for (const auto& stop: catalogue.stops_()){
            geo::Coordinates c;
            c.lng = stop.longitude();
            c.lat = stop.latitude();
            std::string name = stop.name();
            transport_catalogue::Stop s(name, c);
            tc.AddStop(s);
        }
        for (const auto& dist : catalogue.distanses()){
            tc.SetDistance(dist.destination(), dist.departure(), dist.distance());
        }
        for (const auto& bus : catalogue.buses_()){
            std::vector<transport_catalogue::Stop*> stops;
            for(const auto& stop: bus.stops()){
                std::string name = stop.name();
                transport_catalogue::Stop* s = tc.GetStopPointer(name);
                stops.push_back(s);
            }
            std::string name = bus.name(); 
            transport_catalogue::Bus b(name, stops, bus.is_roundtrip());
            tc.AddBus(b);
        }    
    }

    renderer::MapRenderer DeserializeMapRender(const proto::TransportCatalogue& catalogue){
        renderer::RenderingSettings buffer;
        buffer.width = catalogue.render_settings().width();
        buffer.height = catalogue.render_settings().height();
        buffer.padding = catalogue.render_settings().padding();
        buffer.line_width = catalogue.render_settings().line_width();
        buffer.stop_radius = catalogue.render_settings().stop_radius();
        buffer.bus_label_font_size = catalogue.render_settings().bus_label_font_size();
        buffer.underlayer_width = catalogue.render_settings().underlayer_width();

        renderer::LabelOffset bus_label_offset = {catalogue.render_settings().bus_label_offset().Get(0) , catalogue.render_settings().bus_label_offset().Get(1)};
        buffer.bus_label_offset = bus_label_offset;

        buffer.stop_label_font_size = catalogue.render_settings().stop_label_font_size();
        renderer::LabelOffset stop_label_offset = {catalogue.render_settings().stop_label_offset().Get(0), catalogue.render_settings().stop_label_offset().Get(1)};
        buffer.stop_label_offset = stop_label_offset;

        for (const auto& color : catalogue.render_settings().color_palette()){
            if (color.color_case() == 2){
                buffer.color_palette.push_back(svg::Color(color.color_name()));
            }
            else if (color.color_case() == 1){
                proto::RGBA rgba = color.rgba();
                svg::Rgba rgba_color{static_cast<uint8_t>(rgba.r()), static_cast<uint8_t>(rgba.g()), static_cast<uint8_t>(rgba.b()), rgba.opacity()};
                buffer.color_palette.push_back(svg::Color(rgba_color));
            }
            else if (color.color_case() == 3){
                proto::RGB rgb = color.rgb();
                svg::Rgb rgb_color{static_cast<uint8_t>(rgb.r()), static_cast<uint8_t>(rgb.g()), static_cast<uint8_t>(rgb.b())};
                buffer.color_palette.push_back(svg::Color(rgb_color));
            }
        }
        
        if (catalogue.render_settings().underlayer_color().color_case() == 2){
            buffer.underlayer_color = svg::Color(catalogue.render_settings().underlayer_color().color_name());
        }
        else if (catalogue.render_settings().underlayer_color().color_case() == 1){
            proto::RGBA rgba = catalogue.render_settings().underlayer_color().rgba();
            svg::Rgba rgba_color{static_cast<uint8_t>(rgba.r()), static_cast<uint8_t>(rgba.g()), static_cast<uint8_t>(rgba.b()), rgba.opacity()};
            buffer.underlayer_color = rgba_color;
        }
        else if (catalogue.render_settings().underlayer_color().color_case() == 3){
            proto::RGB rgb = catalogue.render_settings().underlayer_color().rgb();
            svg::Rgb rgb_color{static_cast<uint8_t>(rgb.r()), static_cast<uint8_t>(rgb.g()), static_cast<uint8_t>(rgb.b())};
            buffer.underlayer_color = rgb_color;
        }
        return renderer::MapRenderer(buffer);
    }

    void ProcessRequests(const std::string& db, const json::Document& doc){        
        proto::TransportCatalogue catalogue;
        std::ifstream is(db.data(), std::ios::binary);
        catalogue.ParseFromIstream(&is);
        is.close();
        json::Array queries_to_respond = doc.GetRoot().AsMap().at("stat_requests").AsArray();
        std::vector<json::Node> result;
        transport_catalogue::TransportCatalogue tc_;
        SetTransportCatalogue(catalogue, tc_);
        renderer::MapRenderer renderer = DeserializeMapRender(catalogue);
        router::TransportRouter router = DeserializeTransportRouter(catalogue, tc_);
        catalogue.Clear();
        for (const auto& item : queries_to_respond){
            const auto resp_map = item.AsMap();
            if (resp_map.at("type").AsString() == "Stop"){
                result.push_back(json_reader::GetStopOutput(resp_map.at("name").AsString(), resp_map.at("id").AsInt(), tc_));
            }
            if (resp_map.at("type").AsString() == "Bus"){
                result.push_back(json_reader::GetBusOutput(resp_map.at("name").AsString(), resp_map.at("id").AsInt(), tc_));
            }
            if (resp_map.at("type").AsString() == "Map"){
                result.push_back(json_reader::GetMapOutput(resp_map.at("id").AsInt(), renderer, tc_));
            }
            if (resp_map.at("type").AsString() == "Route"){
                result.push_back(json_reader::GetRouteOutput(resp_map.at("id").AsInt(), resp_map.at("from").AsString(), resp_map.at("to").AsString(), router, tc_));
            }
        }
        json::Print(json::Document(json::Builder{}.Value(result).Build()), std::cout);

    }
}
