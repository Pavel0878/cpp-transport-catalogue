#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */


using namespace std;

void ParseStop(TransportCatalogue& tc, const json::Dict& base) {
    string name = base.at("name"s).AsString();
    tc.AddStop(name
        , base.at("latitude"s).AsDouble()
        , base.at("longitude"s).AsDouble());
    //cout << base.at("name"s).AsString() << endl;
}

void ParseDistance(TransportCatalogue& tc, const json::Dict& base) {
    string Stop1 = base.at("name"s).AsString();
    json::Dict str = base.at("road_distances"s).AsMap();
    for (auto [Stop2, node] : str) {
        tc.AddDistanceStop(Stop1, Stop2, node.AsInt());
       // cout << Stop1 << ','<< Stop2 << ':'<<node.AsInt()<<endl;
    }
}

void ParseBus(TransportCatalogue& tc, const json::Dict& base) {
    auto stops = base.at("stops"s).AsArray();
    vector <string_view> bus_stops;
    for (const auto node : stops) {
        bus_stops.push_back(tc.FindStop(node.AsString())->stop_name);
       // cout << node.AsString() << ',' << endl;
    }
    tc.AddBus(base.at("name"s).AsString()
        , bus_stops
        , base.at("is_roundtrip"s).AsBool());
    //cout << "is_roundtrip"s << base.at("is_roundtrip"s).AsBool() << endl;
}

void BaseParse(TransportCatalogue& tc, const json::Array& arr) {
    for(const auto node : arr){
        auto dict = node.AsMap();
        if (dict.at("type"s).AsString() == "Stop"s) {
            ParseStop(tc, dict);
        }
     }

    for (const auto node : arr) {
        json::Dict dict = node.AsMap();
        string type = dict.at("type"s).AsString();
        if (type == "Stop"s) {
            ParseDistance(tc, dict);
        }
        if (type == "Bus"s) {
            ParseBus(tc, dict);
        }
    }
}

void Read(istream& is, ostream& out){
	TransportCatalogue tc;
    ostringstream out_svg;
	const json::Document document = json::Load(is);
    auto str = document.GetRoot().AsMap();
    if (str.count("base_requests"s)) {
        BaseParse(tc, str.at("base_requests"s).AsArray());
    }
    if (str.count("render_settings"s)) {
        SvgRead(tc, str.at("render_settings"s).AsMap(), out_svg);
    }
    if (str.count("stat_requests"s)) {
        json::Dict dic;
        json::Node svg_out(out_svg.str());
        dic["map"s] = svg_out;
        json::Print(json::Document(InfoParse(tc, str.at("stat_requests"s).AsArray(), dic)),out);
    }
}
