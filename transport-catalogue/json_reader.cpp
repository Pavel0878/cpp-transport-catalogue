#include "json_reader.h"

using namespace std;

svg::Color ConvertColor(const json::Node colors) {
    if (colors.IsArray()) {
        auto color = colors.AsArray();
        if (color.size() == 3) {
            uint8_t r = static_cast<uint8_t>(color[0].AsDouble());
            uint8_t g = static_cast<uint8_t>(color[1].AsDouble());
            uint8_t b = static_cast<uint8_t>(color[2].AsDouble());
            return svg::Rgb{ r, g, b };
        }
        else {
            uint8_t r = static_cast<uint8_t>(color[0].AsDouble());
            uint8_t g = static_cast<uint8_t>(color[1].AsDouble());
            uint8_t b = static_cast<uint8_t>(color[2].AsDouble());
            double opacity = color[3].AsDouble();
            return svg::Rgba{ r, g, b, opacity };
        }
    }
    return colors.AsString();
}

void JsonReader::Read(istream& is, ostream& out) {
    const json::Document document = json::Load(is);
    const json::Dict dict = document.GetRoot().AsDict();
    if (dict.count("base_requests"s)) {
        SetBase(dict.at("base_requests"s).AsArray());
    }
    if (dict.count("render_settings"s)) {
        SetMap(dict.at("render_settings"s).AsDict(), map_out_);
    }
    if (dict.count("stat_requests"s)) {
        json::Array arr_out = GetInfo(dict.at("stat_requests"s).AsArray());
        json::Print(
            json::Document{
            json::Builder{}
            .Value(arr_out)
                .Build()
        }, out);
       // auto tp = dict.at("stat_requests"s).AsArray();
      //  json::Print(json::Document(GetInfo(dict.at("stat_requests"s).AsArray())), out);
    }
}

void JsonReader::SetBase(const json::Array& arr) {
    for (const auto node : arr) {
        auto dict = node.AsDict();
        if (dict.at("type"s).AsString() == "Stop"s) {
            SetStop(dict);
        }
    }

    for (const auto node : arr) {
        json::Dict dict = node.AsDict();
        string type = dict.at("type"s).AsString();
        if (type == "Stop"s) {
            SetDistance(dict);
        }
        if (type == "Bus"s) {
            SetBus(dict);
        }
    }
}

void JsonReader::SetStop(const json::Dict& dict) {
    string name = dict.at("name"s).AsString();
    tc_.AddStop(name
        , dict.at("latitude"s).AsDouble()
        , dict.at("longitude"s).AsDouble());
}

void JsonReader::SetDistance(const json::Dict& dict) {
    string Stop1 = dict.at("name"s).AsString();
    json::Dict str = dict.at("road_distances"s).AsDict();
    for (auto [Stop2, node] : str) {
        tc_.AddDistanceStop(Stop1, Stop2, node.AsInt());
    }
}

void JsonReader::SetBus(const json::Dict& dict) {
    auto stops = dict.at("stops"s).AsArray();
    vector <string_view> bus_stops;
    for (const auto node : stops) {
        bus_stops.push_back(tc_.FindStop(node.AsString())->stop_name);
    }
    tc_.AddBus(dict.at("name"s).AsString()
        , bus_stops
        , dict.at("is_roundtrip"s).AsBool());
}

void JsonReader::SetMap(const json::Dict& dict, ostream& out) {

    renderer::Map map;
    map.width = dict.at("width").AsDouble();
    map.height = dict.at("height").AsDouble();
    map.padding = dict.at("padding").AsDouble();
    map.stop_radius = dict.at("stop_radius"s).AsDouble();
    map.line_width = dict.at("line_width"s).AsDouble();
    map.bus_label_font_size = dict.at("bus_label_font_size"s).AsInt();
    auto bus_label = dict.at("bus_label_offset"s).AsArray();
    map.bus_label_offset = svg::Point(bus_label[0].AsDouble(), bus_label[1].AsDouble());
    map.stop_label_font_size = dict.at("stop_label_font_size"s).AsInt();
    auto stop_label = dict.at("stop_label_offset"s).AsArray();
    map.stop_label_offset = svg::Point(stop_label[0].AsDouble(), stop_label[1].AsDouble());
    map.underlayer_color = ConvertColor(dict.at("underlayer_color"s));
    map.underlayer_width = dict.at("underlayer_width"s).AsDouble();
    auto palette_color = dict.at("color_palette"s).AsArray();
    for (auto color : palette_color) {
        map.color_palette.emplace_back(ConvertColor(color));
    }
    renderer::MapRenderer mr_(map);

    vector<TransportCatalogue::Bus> bus_all;
    for (auto bus : tc_.GetAllBus()) {
        bus_all.emplace_back(*tc_.FindBus(bus));
    }
    mr_.SetRoute(bus_all, tc_.GetStops());
    mr_.GetDocument(out);
}

json::Array JsonReader::GetInfo(const json::Array& arr) {
    json::Array result;
    for (const auto node : arr) {
        auto dict = node.AsDict();
        if (dict.at("type"s).AsString() == "Stop"s) {
            result.emplace_back(GetStop(dict));
        }
        if (dict.at("type"s).AsString() == "Bus"s) {
            result.emplace_back(GetBus(dict));
        }
        if (dict.at("type"s).AsString() == "Map"s) {
            result.emplace_back(GetMap(dict));
        }
    }
    return result;
}

json::Dict JsonReader::GetStop(const json::Dict& dict) {
    json::Dict result;
    json::Array arr;
    auto str = tc_.GetStopInfo(dict.at("name"s).AsString());
    if (dict.count("id"s)) {
        result.emplace("request_id"s, dict.at("id"s).AsInt());
    }
    if (str == nullopt) {
        result.emplace("error_message"s, json::Node("not found"s));
    }
    else {
        for (auto& st : *str) {
            arr.push_back(string(st));
        }
        result.emplace("buses", arr);
    }

    return result;
}

json::Dict JsonReader::GetBus(const json::Dict& dict) {
    json::Dict result;
    auto bus_info = tc_.GetBusInfo(dict.at("name"s).AsString());
    result.emplace("request_id"s, dict.at("id"s).AsInt());
    if (bus_info.unique_stops == 0) {
        result.emplace("error_message"s, json::Node("not found"s));
    }
    else {
        result.emplace("curvature"s, json::Node(bus_info.real_number));
        result.emplace("route_length"s, json::Node(bus_info.actual_length));
        result.emplace("stop_count"s, json::Node(static_cast<int>(bus_info.stops_on_route)));
        result.emplace("unique_stop_count"s, json::Node(static_cast<int>(bus_info.unique_stops)));
    }
    return result;
}

json::Dict JsonReader::GetMap(const json::Dict& dict) {
    int id = dict.at("id"s).AsInt();
    return json::Builder{}
    .StartDict()
        .Key("request_id"s).Value(id)
        .Key("map"s).Value(map_out_.str())
        .EndDict()
        .Build()
        .AsDict();

   // json::Dict result;
   // json::Node doc_svg(out_svg_.str());
   // result.emplace("request_id"s, dict.at("id"s).AsInt());
  //  result.emplace("map"s, doc_svg);
  //  return result;
}