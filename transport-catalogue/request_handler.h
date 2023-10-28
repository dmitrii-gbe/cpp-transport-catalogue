#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "transport_catalogue.h"
#include "geo.h"
#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace transport_catalogue {
    namespace request_handler{

class RequestHandler {
public:

    RequestHandler(TransportCatalogue& db, const renderer::MapRenderer& renderer, const router::TransportRouter& router);

    void FillDB(const json::Document& doc);
    
    void RespondToRequest(const json::Document& doc, std::ostream& out) const;



private:    
    
    TransportCatalogue& tc_;
    const renderer::MapRenderer& renderer_;
    const router::TransportRouter& router_;
};
    }
}


