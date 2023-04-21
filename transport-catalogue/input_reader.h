#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <stdexcept>
#include <algorithm>
#include <deque>
#include <list>
#include <vector>
#include <unordered_map>


#include "geo.h"

namespace transport_catalogue {
    class TransportCatalogue;

    int ReadLineWithNumber(std::istream &is);

// trim from start (in place)
    void ltrim(std::string &s);

// trim from end (in place)
    void rtrim(std::string &s);

// trim from both ends (in place)
    void trim(std::string &s);

    void ltrim_v(std::string_view &s);

    void rtrim_v(std::string_view &s);

    void trim_v(std::string_view &s);


    std::vector<std::string_view> SplitStrToStrV(std::string_view s, char split_char, int piece_num = 0);


    class InputReader {

    public:
        struct InputBusInfo {
            std::string bus_name;
            std::deque<std::string_view> stops;
        };

        struct InputStopInfo {
            std::string stop_name;
            geo::Coordinates coordinates;
        };

        struct InputCommand {
            std::string command;
            std::string first_parameter;
            std::string second_parameter;
        };

        struct InputDistanceInfo {
            std::string_view stop_name;
            std::unordered_map<std::string_view, size_t> distance_to_neighbour;
        };

        explicit InputReader(std::istream &is) : input_stream_(is) {}

        void ParseRequests();

        InputStopInfo *ParseStopInfo(const InputCommand &parsed_command);

        InputBusInfo *ParseBusInfo(const InputCommand &parsed_command);

        void UpdateTcData(TransportCatalogue &tc);

        InputCommand ParseInputCommand(const std::string &str);

    private:
        std::istream &input_stream_;
        std::deque<InputStopInfo> parsed_stop_info_deque_;
        std::deque<InputBusInfo> parsed_bus_info_deque_;
        std::deque<InputDistanceInfo> parsed_distance_info_deque_;
        std::list<InputCommand> input_commands_;
    };
}