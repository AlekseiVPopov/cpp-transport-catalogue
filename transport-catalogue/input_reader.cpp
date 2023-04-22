#include "input_reader.h"
#include "transport_catalogue.h"

namespace transport_catalogue {

    std::string ReadLine() {
        std::string s;
        std::getline(std::cin, s);
        return s;
    }

    int ReadLineWithNumber(std::istream &is) {
        int result;
        is >> result;
        ReadLine();
        return result;
    }

// trim from start (in place)
    void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

// trim from end (in place)
    void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

// trim from both ends (in place)
    void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }

// trim from start (in place)
    void ltrim_v(std::string_view &s) {
        auto pos = s.find_first_not_of(' ');
        if (pos != std::string_view::npos) { s.remove_prefix(s.find_first_not_of(' ')); }
    }

// trim from end (in place)
    void rtrim_v(std::string_view &s) {
        auto pos = s.find_last_not_of(' ');

        if (pos != std::string_view::npos) { s.remove_suffix(s.length() - s.find_last_not_of(' ') - 1); }
    }


// trim from both ends (in place)
    void trim_v(std::string_view &s) {
        ltrim_v(s);
        rtrim_v(s);

    }

    std::vector<std::string_view> SplitStrToStrV(std::string_view str, char split_char, int piece_num) {
        std::vector<std::string_view> result;
        const auto end_pos = std::string_view::npos;

        trim_v(str);

        if (!piece_num) {
            while (true) {
                auto delim_pos = str.find(split_char);
                result.push_back(delim_pos == end_pos ? str.substr(0) : str.substr(0, delim_pos));
                if (delim_pos == end_pos) {
                    break;
                } else {
                    str.remove_prefix(delim_pos + 1);
                }
            }
        } else {
            for (int i = 1; i < piece_num; ++i) {
                auto delim_pos = str.find(split_char);
                result.push_back(delim_pos == end_pos ? str.substr(0) : str.substr(0, delim_pos));
                if (delim_pos == end_pos) {
                    break;
                } else {
                    str.remove_prefix(delim_pos + 1);
                }
            }
            result.push_back(str.substr(0));
        }
        return result;
    }

    InputCommand InputReader::ParseInputCommand(const std::string &str) {
        InputCommand result;
        if (str.empty()) {
            return result;
        }

        const auto end_pos = std::string::npos;
        auto delim_pos = str.find(':');

        if (delim_pos != end_pos) {
            auto left_part = str.substr(0, delim_pos);
            trim(left_part);
            auto right_part = str.substr(delim_pos + 1);
            trim(right_part);
            auto command_pos = str.find_first_of(' ');
            result.command = left_part.substr(0, command_pos);
            result.first_parameter = left_part.substr(command_pos + 1);
            trim(result.first_parameter);
            result.second_parameter = right_part;
        }
        return result;
    }


    InputStopInfo *InputReader::ParseStopInfo(const InputCommand &parsed_command) {

        auto parsed_parameters = SplitStrToStrV(parsed_command.second_parameter, ',');

        auto lat = std::stod(std::string(parsed_parameters[0]));
        auto lng = std::stod(std::string(parsed_parameters[1]));

        std::unordered_map<std::string_view, size_t> dist_map;

        if (parsed_parameters.size() > 2) {
            for (int i = 2; i < parsed_parameters.size(); ++i) {
                auto dist = SplitStrToStrV(parsed_parameters[i], ' ', 3);
                dist[0].remove_suffix(1);
                dist_map[dist[2]] = std::stol(std::string(dist[0]));
            }
            parsed_distance_info_deque_.emplace_back(
                    std::move(InputDistanceInfo{parsed_command.first_parameter, std::move(dist_map)}));
        }

        auto &new_stop = parsed_stop_info_deque_.emplace_back(
                std::move(InputStopInfo{parsed_command.first_parameter, {lat, lng}}));

        return &new_stop;
    }

    InputBusInfo *InputReader::ParseBusInfo(const InputCommand &parsed_command) {
        using namespace std::string_literals;


        auto delim_circled_pos = parsed_command.second_parameter.find('-');
        auto delim_manual_pos = parsed_command.second_parameter.find('>');

        if (delim_circled_pos == delim_manual_pos) {
            throw std::runtime_error("No stops delimiter in Bus info "s + parsed_command.first_parameter + " " +
                                     parsed_command.second_parameter);
        }

        bool circled = delim_circled_pos != std::string_view::npos;

        char delim_char = circled ? '-' : '>';
        std::vector<std::string_view> stops_vec(SplitStrToStrV(parsed_command.second_parameter, delim_char));

        for (auto &stop: stops_vec) {
            trim_v(stop);
        }

        std::deque<std::string_view> stops_v_deque{stops_vec.begin(), stops_vec.end()};

        if (circled) {
            for (auto r_it = next(stops_vec.rbegin()); r_it != stops_vec.rend(); ++r_it) {
                stops_v_deque.push_back(*r_it);
            }
        }

        if (!stops_v_deque.empty() && stops_v_deque.front() != stops_v_deque.back()) {
            throw std::runtime_error("Empty route or not circled, Bus "s + parsed_command.first_parameter);
        }

        auto &new_bus = parsed_bus_info_deque_.emplace_back(
                std::move(InputBusInfo{parsed_command.first_parameter, stops_v_deque}));

        return &new_bus;
    }

    void InputReader::ParseRequests() {
        using namespace std::string_literals;

        int requests_num = ReadLineWithNumber(input_stream_);

        std::string input_string;


        while (requests_num--) {
            std::getline(input_stream_, input_string);

            trim(input_string);

            if (std::count(input_string.begin(), input_string.end(), ':') != 1) {
                throw std::runtime_error("Input request string have wrong format (no : char) - " + input_string);
            }


            auto &parsed_command = input_commands_.emplace_back(std::move(ParseInputCommand(input_string)));

            if (parsed_command.command == "Stop"s) {
                ParseStopInfo(parsed_command);
            } else if (parsed_command.command == "Bus"s) {
                ParseBusInfo(parsed_command);
            } else {
                throw std::runtime_error("Input request string have unknown command  - " +
                                         parsed_command.command + " " +
                                         parsed_command.first_parameter + " " +
                                         parsed_command.second_parameter);
            }
        }
    }

    void InputReader::UpdateTcData(TransportCatalogue &tc) {

        for (auto &stop: parsed_stop_info_deque_) {
            tc.AddStop(&stop);
        }

        for (auto &bus: parsed_bus_info_deque_) {
            tc.AddBus(&bus);
        }

        for (auto &dist: parsed_distance_info_deque_) {
            tc.AddRealDistance(&dist);
        }
    }
}