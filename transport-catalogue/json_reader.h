#pragma once

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

namespace json_reader {
json::Document GetQuery(std::istream& input);

renderer::RenderingSettings GetRenderingSettings(const json::Document& document);

void GetColorSettings(renderer::RenderingSettings& result, const json::Dict& settings);

void AddStops(const json::Array& queries_to_add, transport_catalogue::TransportCatalogue& tc);
void AddBuses(const json::Array& queries_to_add, transport_catalogue::TransportCatalogue& tc);
void SetDistances(const json::Array& queries_to_add, transport_catalogue::TransportCatalogue& tc);

void FillDB(const json::Document& doc, transport_catalogue::TransportCatalogue& tc);

json::Node BusOutput(const std::string& name, int id, const transport_catalogue::TransportCatalogue& tc);
json::Node StopOutput(const std::string& name, int id, const transport_catalogue::TransportCatalogue& tc);

void RenderRoutesMap(std::ostream& out, const renderer::MapRenderer& renderer, const transport_catalogue::TransportCatalogue& tc);

json::Node MapOutPut(const int request_id, const renderer::MapRenderer& renderer, const transport_catalogue::TransportCatalogue& tc);

}
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */