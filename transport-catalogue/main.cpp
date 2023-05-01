#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"


int main() {

    using namespace transport_catalogue;
    using namespace json_reader;
    using namespace json;
    using namespace renderer;

//    const double WIDTH = 600.0;
//    const double HEIGHT = 400.0;
//    const double PADDING = 50.0;
//
//    // Точки, подлежащие проецированию
//    std::vector<geo::Coordinates> geo_coords = {
//            {43.587795, 39.716901}, {43.581969, 39.719848}, {43.598701, 39.730623},
//            {43.585586, 39.733879}, {43.590317, 39.746833}
//    };
//
//    // Создаём проектор сферических координат на карту
//    const SphereProjector proj{
//            geo_coords.begin(), geo_coords.end(), WIDTH, HEIGHT, PADDING
//    };
//
//    // Проецируем и выводим координаты
//    for (const auto geo_coord: geo_coords) {
//        const svg::Point screen_coord = proj(geo_coord);
//        std::cout << '(' << geo_coord.lat << ", "sv << geo_coord.lng << ") -> "sv;
//        std::cout << '(' << screen_coord.x << ", "sv << screen_coord.y << ')' << std::endl;
//    }



    TransportCatalogue tc;
    svg::Document doc;
    MapRenderer mr(doc);

    RequestHandler rh(tc, mr);
    //RequestHandler rh(tc);



    auto input_json = LoadJSONStream(std::cin);
    PushInputJsonToRH(input_json, rh);

    rh.ParseBaseRequests();

    rh.PushBaseRequest();
    std::cout << rh.PushStatRequests();

    //rh.GenerateMapToSvg();



//    auto input_json = LoadJSONStream(std::cin).GetRoot().AsMap();
//    auto s = Print(input_json);
//
//    std::cout << s << std::endl;

//    TransportCatalogue tc;
//
//
//    InputReader input_reader(std::cin);
//    input_reader.ParseBaseRequests();
//    input_reader.UpdateTcData(tc);
//
//
//    StatReader stat_reader(std::cin);
//    stat_reader.ParseBaseRequests();
//    stat_reader.Output(tc);

//
//    Stop stop1{"Tolstopaltsevo"s, 55.611087, 37.208290};
//    Stop stop2{"Marushkino"s, 55.595884, 37.209755};
//    Stop stop3{"Rasskazovka"s, 55.632761, 37.333324};
//    Stop stop4{"Biryulyovo Zapadnoye"s, 55.574371, 37.651700};
//    Stop stop5{"Biryusinka"s, 55.581065, 37.648390};
//    Stop stop6{"Universam"s, 55.587655, 37.645687};
//    Stop stop7{"Biryulyovo Tovarnaya"s, 55.592028, 37.653656};
//    Stop stop8{"Biryulyovo Passazhirskaya"s, 55.580999, 37.659164};
//
//    tc.AddStop(&stop1);
//    tc.AddStop(&stop2);
//    tc.AddStop(&stop3);
//    tc.AddStop(&stop4);
//    tc.AddStop(&stop5);
//    tc.AddStop(&stop6);
//    tc.AddStop(&stop7);
//    tc.AddStop(&stop8);
//
//    std::deque<Stop *> bus256_stops{tc.FindStop("Biryulyovo Zapadnoye"s),
//                                    tc.FindStop("Biryusinka"s),
//                                    tc.FindStop("Universam"s),
//                                    tc.FindStop("Biryulyovo Tovarnaya"s),
//                                    tc.FindStop("Biryulyovo Passazhirskaya"s),
//                                    tc.FindStop("Biryulyovo Zapadnoye"s)};
//    std::deque<std::string_view> bus256_stops_view;
//
//    for (auto s : bus256_stops) {
//        bus256_stops_view.push_back(s->stop_name);
//    }
//
//    Bus bus256{"256"s, bus256_stops_view};
//
//    std::deque<std::string_view> bus750_stops{"Tolstopaltsevo"s, "Marushkino"s, "Rasskazovka"s, "Marushkino"s,
//                                              "Tolstopaltsevo"s};
//    Bus bus750{"750"s, std::move(bus750_stops)};
//
//    tc.AddBus(&bus256);
//    tc.AddBus(&bus750);
//
//    auto f256 = tc.FindBus("256"s);
//    auto f750 = tc.FindBus("750"s);
//    auto f751 = tc.FindBus("751"s);
//
//    auto resp = tc.GetBusInfo("256"s);
//
//    transport_catalogue::StatReader stat_reader(std::cin);
//    stat_reader.Output(tc);

    return 0;
}