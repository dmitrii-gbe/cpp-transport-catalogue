#include "request_handler.h"
#include <iomanip>
#include <algorithm>

namespace transport_catalogue {
    namespace request_handler {

RequestHandler::RequestHandler(TransportCatalogue& db, const renderer::MapRenderer& renderer) : tc_(db), renderer_(renderer) 
{
}

void RequestHandler::RespondToRequest(const json::Document& doc, std::ostream& out) const{
    json_reader::RespondToRequest(doc, out, renderer_, tc_);
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