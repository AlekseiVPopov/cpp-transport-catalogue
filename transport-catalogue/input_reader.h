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
#include "transport_catalogue.h"

namespace transport_catalogue {

    struct InputCommand {
        std::string command;
        std::string first_parameter;
        std::string second_parameter;
    };


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