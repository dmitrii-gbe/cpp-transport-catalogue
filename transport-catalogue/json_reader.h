#pragma once

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "json_builder.h"
#include "transport_router.h"

namespace json_reader {

const double VELOCITY_TO_METERS_PER_MIN = 1000.0 / 60.0;

std::istringstream ReadStream();

json::Document GetQuery(std::istream& input);

renderer::RenderingSettings GetRenderingSettings(const json::Document& document);

router::RouterSettings GetRouterSettings(const json::Document& document);
json::Node GetRouteOutput(int id, const std::string& from, const std::string& to, const router::TransportRouter& router, const transport_catalogue::TransportCatalogue& tc_);
std::pair<json::Node, double> BuildRouteNode(const std::optional<router::RouteSegments>& route);

void GetColorSettings(renderer::RenderingSettings& result, const json::Dict& settings);

void AddStops(const json::Array& queries_to_add, transport_catalogue::TransportCatalogue& tc);
void AddBuses(const json::Array& queries_to_add, transport_catalogue::TransportCatalogue& tc);
void SetDistances(const json::Array& queries_to_add, transport_catalogue::TransportCatalogue& tc);

void FillDB(const json::Document& doc, transport_catalogue::TransportCatalogue& tc);

json::Node GetBusOutput(const std::string& name, int id, const transport_catalogue::TransportCatalogue& tc);
json::Node GetStopOutput(const std::string& name, int id, const transport_catalogue::TransportCatalogue& tc);

void RenderRoutesMap(std::ostream& out, const renderer::MapRenderer& renderer, const transport_catalogue::TransportCatalogue& tc);

json::Node GetMapOutput(const int request_id, const renderer::MapRenderer& renderer, const transport_catalogue::TransportCatalogue& tc);

void RespondToRequest(const json::Document& doc, std::ostream& out, const renderer::MapRenderer& renderer_, const transport_catalogue::TransportCatalogue& tc_, const router::TransportRouter& router);

}
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */