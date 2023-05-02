#pragma once

#include <sstream>

#include "json.h"
#include "request_handler.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace transport_catalogue {

    json::Document LoadJSONString(const std::string &s);
    json::Document LoadJSONStream(std::istream& input);

    void PushInputJsonToRH(json::Document &json_doc, transport_catalogue::RequestHandler& rh);


    std::string Print(const json::Node &node);

}