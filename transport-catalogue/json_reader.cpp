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
    if (dict.count("routing_settings"s)) {
        SetRoutingSettings(dict.at("routing_settings"s).AsDict());
    }
    if (dict.count("stat_requests"s)) {
        json::Print(
            json::Document{
            json::Builder{}
            .Value(GetInfo(dict.at("stat_requests"s).AsArray()))
                .Build()
        }, out);
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
    for (const auto& bus : tc_.GetAllBus()) {
        bus_all.emplace_back(*tc_.FindBus(bus));
    }
    mr_.SetRoute(bus_all, tc_.GetStops());
    mr_.GetDocument(out);
}

void JsonReader::SetRoutingSettings(const json::Dict& dict) {
    TransportRouter::RoutingSettings routing_settings;
    routing_settings.bus_velocity = dict.at("bus_velocity"s).AsDouble();
    routing_settings.bus_wait_time = dict.at("bus_wait_time"s).AsDouble();
    tr_.SetRoutingSettings(tc_, routing_settings);
}

json::Array JsonReader::GetInfo(const json::Array& arr) {
    json::Array result;
    for (const auto node : arr) {
        json::Dict dict = node.AsDict();
        string type = dict.at("type"s).AsString();
        if (type == "Stop"s) {
            result.emplace_back(GetStop(dict));
        }
        if (type == "Bus"s) {
            result.emplace_back(GetBus(dict));
        }
        if (type == "Map"s) {
            result.emplace_back(GetMap(dict));
        }
        if (type == "Route"s) {
            result.emplace_back(GetRoute(dict));
        }
    }
    return result;
}

json::Dict JsonReader::GetError(const int id) {
    return json::Builder{}
    .StartDict()
        .Key("request_id"s).Value(id)
        .Key("error_message"s).Value("not found"s)
        .EndDict()
        .Build()
        .AsDict();
}

json::Dict JsonReader::GetStop(const json::Dict& dict) {
    auto str = tc_.GetStopInfo(dict.at("name"s).AsString());
    int id = dict.at("id"s).AsInt();
    if (str == nullopt) {
        return GetError(id);
    }
    else {
        json::Array arr;
        for (auto& st : *str) {
            arr.push_back(string(st));
        }
        return json::Builder{}
        .StartDict()
            .Key("request_id"s).Value(id)
            .Key("buses").Value(arr)
            .EndDict()
            .Build()
            .AsDict();
    }
}

json::Dict JsonReader::GetBus(const json::Dict& dict) {
    auto bus_info = tc_.GetBusInfo(dict.at("name"s).AsString());
    int id = dict.at("id"s).AsInt();
    if (bus_info.unique_stops == 0) {
        return GetError(id);
    }
    else {
        return json::Builder{}
        .StartDict()
            .Key("request_id"s).Value(id)
            .Key("curvature"s).Value(bus_info.real_number)
            .Key("route_length"s).Value(bus_info.actual_length)
            .Key("stop_count"s).Value(static_cast<int>(bus_info.stops_on_route))
            .Key("unique_stop_count"s).Value(static_cast<int>(bus_info.unique_stops))
            .EndDict()
            .Build()
            .AsDict();
    }
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
}

json::Dict JsonReader::GetRoute(const json::Dict& dict) {
    int id = dict.at("id"s).AsInt();
    auto& rout = tr_.BuildTransportRouter(
        dict.at("from"s).AsString(),
        dict.at("to"s).AsString()
    ); 

    json::Array items;
    double total_time = .0;
    auto& graph = tr_.GetGraph();

    if (rout) {

        for (auto& edge_id : rout.value().edges) {

            auto& edge = graph.GetEdge(edge_id);
            if (edge.stop_count == 0) {
                items.emplace_back(json::Node(
                    json::Builder{}
                .StartDict()
                    .Key("stop_name"s).Value(edge.name)
                    .Key("time"s).Value(edge.weight)
                    .Key("type"s).Value("Wait"s)
                    .EndDict()
                    .Build()));

                total_time += edge.weight;
            }
            else {
                items.emplace_back(json::Node(
                    json::Builder{}
                .StartDict()
                    .Key("bus"s).Value(edge.name)
                    .Key("span_count"s).Value(static_cast<double>(edge.stop_count))
                    .Key("time"s).Value(edge.weight)
                    .Key("type"s).Value("Bus"s)
                    .EndDict()
                    .Build()));
                total_time += edge.weight;
            }
        }
            return json::Builder{}
            .StartDict()
                .Key("request_id"s).Value(id)
                .Key("total_time"s).Value(total_time)
                .Key("items"s).Value(items)
                .EndDict()
                .Build()
                .AsDict();
        
    }
    return GetError(id);
}