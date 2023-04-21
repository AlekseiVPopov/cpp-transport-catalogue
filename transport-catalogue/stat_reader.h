#pragma once

#include <iostream>
#include <string>
#include <set>
#include <algorithm>
#include <deque>
#include <iomanip>


#include "transport_catalogue.h"
#include "input_reader.h"

namespace transport_catalogue {



    class StatReader {
    public:

        struct DataRequest {
            std::string command;
            std::string request;
        };


        explicit StatReader(std::istream &is) : input_stream_(is) {};

        void ParseRequests();

        void Output(class TransportCatalogue &tc);

    private:
        std::istream &input_stream_;
        std::deque<DataRequest> data_requests_;
    };

}