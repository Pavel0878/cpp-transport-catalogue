#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

using namespace std;



json::Dict InfoStop(TransportCatalogue& tc, json::Dict dict) {
	json::Dict result;
	json::Array arr;
	auto str = tc.GetStopInfo(dict.at("name"s).AsString());
	if (dict.count("id"s)) {
		result.emplace("request_id"s, dict.at("id"s).AsInt());
	}
	if(str == nullopt){
		//|| str->empty()
		result["error_message"] = "not found"s;
		//result.emplace("error_message"s, json::Node("not found"s));
	}
	else {
		for (auto& st : *str) {
			arr.push_back(string(st));
		}
		result.emplace("buses", arr);
	}
	
	return result;
}

json::Dict InfoBus(TransportCatalogue& tc, json::Dict dict) {
	json::Dict result;
	auto bus_info = tc.GetBusInfo(dict.at("name"s).AsString());
	result.emplace("request_id"s, dict.at("id"s).AsInt());
	if (bus_info.unique_stops == 0) {
		//|| str->empty()
		result["error_message"] = "not found"s;
		//result.emplace("error_message"s, json::Node("not found"s));
	}
	else {
		if (bus_info.real_number != 0) {
			result.emplace("curvature"s, json::Node(bus_info.real_number));
		}
		if (bus_info.actual_length != 0) {
			result.emplace("route_length"s, json::Node(bus_info.actual_length));
		}
		if (bus_info.stops_on_route != 0) {
			result.emplace("stop_count"s, json::Node(static_cast<int>(bus_info.stops_on_route)));
		}
		if (bus_info.unique_stops != 0) {
			result.emplace("unique_stop_count"s, json::Node(static_cast<int>(bus_info.unique_stops)));
		}
	}
	return result;
}

json::Array InfoParse(TransportCatalogue& tc, const json::Array arr, json::Dict dic) {
	json::Array result;
	for (const auto node : arr) {
		auto dict = node.AsMap();
		auto str = dict.at("type"s).AsString();
		if (str == "Stop"s) {
			result.emplace_back(InfoStop(tc, dict));
		}
		if (str == "Bus"s) {
			result.emplace_back(InfoBus(tc, dict));
		}
		if (str == "Map"s) {
			dic.emplace("request_id"s, dict.at("id"s).AsInt());
			result.emplace_back(dic);
		}
	}

	return result;
}
