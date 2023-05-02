#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace transport_catalogue {

    using namespace json;
    using namespace transport_catalogue;

    json::Document LoadJSONString(const std::string &s) {
        std::istringstream strm(s);
        return json::Load(strm);
    }

    json::Document LoadJSONStream(std::istream &input) {
        return json::Load(input);
    }


    std::string Print(const json::Node &node) {
        std::ostringstream out;
        Print(json::Document{node}, out);
        return out.str();
    }

    void PushInputJsonToRH(Document &json_doc, RequestHandler &rh) {
        rh.AddRequests(json_doc);
    }

}