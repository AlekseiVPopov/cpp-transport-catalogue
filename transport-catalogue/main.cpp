#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

int main() {

    using namespace transport_catalogue;

    TransportCatalogue tc;


    InputReader input_reader(std::cin);
    input_reader.ParseRequests();
    input_reader.UpdateTcData(tc);


    StatReader stat_reader(std::cin);
    stat_reader.ParseRequests();
    stat_reader.Output(tc);

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