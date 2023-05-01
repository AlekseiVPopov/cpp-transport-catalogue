#pragma once

#include <sstream>

#include "json.h"
#include "request_handler.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json_reader {

    using namespace json;
    using namespace request_handler;

    json::Document LoadJSONString(const std::string &s);
    json::Document LoadJSONStream(std::istream& input);

    void PushInputJsonToRH(Document &json_doc, RequestHandler& rh);


    std::string Print(const json::Node &node);

}