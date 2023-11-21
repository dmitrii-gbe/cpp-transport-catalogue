#pragma once
#include "transport_catalogue.pb.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "json.h"
#include <iostream>

namespace myproto {

    void MakeBase(json::Document queries);

    void ProcessRequests(const std::string& db, const json::Document& queries);

    template <typename T, typename G>
    google::protobuf::Map<T, G> CreateProtoMap (const std::unordered_map<T, G>& map);

    router::RouterSettings SetRouterSettings(const proto::TransportCatalogue& catalogue);

    proto::RGB CreateProtoRGB(const json::Array& array);

    proto::RGBA CreateProtoRGBA(const json::Array& array);

    proto::TransportCatalogue SerializeTransportCatalogue(const transport_catalogue::TransportCatalogue& tc, const json::Document& queries);

    proto::RenderSettings SerializeoRenderingSettings(const json::Dict& render_settings);

    void SetProtoRouter(const router::TransportRouter& router, proto::TransportCatalogue& catalogue);

    router::TransportRouter DeserializeTransportRouter(const proto::TransportCatalogue& catalogue, const transport_catalogue::TransportCatalogue& tc);

    void SetTransportCatalogue(const proto::TransportCatalogue& catalogue, transport_catalogue::TransportCatalogue& tc);

    renderer::MapRenderer DeserializeMapRender(const proto::TransportCatalogue& catalogue);

} // namespace proto end