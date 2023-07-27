#include "request_handler.h"
#include <iomanip>
#include <algorithm>

namespace transport_catalogue {
    namespace request_handler {

RequestHandler::RequestHandler(TransportCatalogue& db, const renderer::MapRenderer& renderer) : tc_(db), renderer_(renderer) 
{
}

void RequestHandler::RespondToRequest(const json::Document& doc, std::ostream& out) const{
    json::Array queries_to_respond = doc.GetRoot().AsMap().at("stat_requests").AsArray();
    std::vector<json::Node> result;
    for (const auto& item : queries_to_respond){
        const auto resp_map = item.AsMap();
        if (resp_map.at("type").AsString() == "Stop"){
            result.push_back(json_reader::StopOutput(resp_map.at("name").AsString(), resp_map.at("id").AsInt(), tc_));
        }
        if (resp_map.at("type").AsString() == "Bus"){
            result.push_back(json_reader::BusOutput(resp_map.at("name").AsString(), resp_map.at("id").AsInt(), tc_));
        }
        if (resp_map.at("type").AsString() == "Map"){
            result.push_back(json_reader::MapOutPut(resp_map.at("id").AsInt(), renderer_, tc_));
        }
    }
    json::Node return_value{result};
    json::Document d(return_value);
    json::Print(d, out);
}

void RequestHandler::FillDB(const json::Document& doc){
    json_reader::FillDB(doc, tc_);
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