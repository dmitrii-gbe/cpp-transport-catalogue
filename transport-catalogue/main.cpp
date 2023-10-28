#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "transport_router.h"
#include <iostream>
#include <iomanip>


int main() {

    json::Document queries = json_reader::GetQuery(std::cin);

    renderer::RenderingSettings settings = json_reader::GetRenderingSettings(queries);

    renderer::MapRenderer mp(settings);
  
    transport_catalogue::TransportCatalogue tc;

    json_reader::FillDB(queries, tc);

    router::RouterSettings router_settings = json_reader::GetRouterSettings(queries);

    router::TransportRouter router(tc, router_settings);

    transport_catalogue::request_handler::RequestHandler rh(tc, mp, router);

    //rh.FillDB(queries);    
    
    rh.RespondToRequest(queries, std::cout);

    return 0;    
}