#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"


int main() {

    using namespace transport_catalogue;
    using namespace json;


    TransportCatalogue tc;
    svg::Document doc;
    MapRenderer mr(doc);

    JsonRequestProcessor rh(tc, mr);

    auto input_json = LoadJSONStream(std::cin);
    PushInputJsonToRH(input_json, rh);

    rh.ParseBaseRequests();
    rh.PushBaseRequest();

    std::cout << rh.PushStatRequests();

    return 0;
}