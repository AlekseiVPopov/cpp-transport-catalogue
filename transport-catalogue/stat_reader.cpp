

#include "stat_reader.h"

namespace transport_catalogue {

    void StatReader::ParseRequests() {

        int requests_num = ReadLineWithNumber(input_stream_);
        std::string input_line;
        std::string command;
        std::string data;


        while (requests_num--) {
            getline(input_stream_, input_line);
            trim(input_line);

            auto space_pos = input_line.find(' ');
            command = input_line.substr(0, space_pos);

            data = input_line.substr(command.size());
            trim(data);
            data_requests_.emplace_back(std::move(DataRequest{command, data}));
        }
    }


    void StatReader::Output(class TransportCatalogue &tc) {
        using namespace std::string_literals;

        for (auto &r: data_requests_) {
            if (r.command == "Bus"s) {
                const auto bus_info = tc.GetBusInfo(r.request);
                //Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length
                if (bus_info.present) {

                    std::cout << r.command << " "s <<
                              r.request << ": "s <<
                              bus_info.stops_num << " stops on route, "s <<
                              bus_info.uniq_stops_num << " unique stops, "s <<
                              std::setprecision(6) <<
                              bus_info.real_length << " route length, "s <<
                              bus_info.curvature << " curvature"s <<
                              std::endl;
                } else {
                    std::cout << r.command << " "s << r.request << ": not found"s << std::endl;
                }
            } else if (r.command == "Stop") {
                const auto stop_info = tc.GetStopInfo(r.request);

                if (stop_info.present) {
                    std::cout << r.command << " "s << r.request;
                    if (!stop_info.buses.empty()) {
                        std::cout << ": buses "s;
                        for (auto bus: stop_info.buses) {
                            std::cout << bus->bus_name << " "s;
                        }
                        std::cout << std::endl;
                    } else {
                        std::cout << ": no buses"s << std::endl;
                    }

                } else {
                    std::cout << r.command << " "s << r.request << ": not found"s << std::endl;
                }
            } else {
                std::cout << "command "s << r.command << " "s << " not found"s << std::endl;
            }

        }
    }
}