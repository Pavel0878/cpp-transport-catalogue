#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"

#include <iostream>
#include <string_view>
#include <string>
#include <vector>
#include <map>
#include <sstream>

class JsonReader {
public:

	void Read(std::istream& is, std::ostream& out);
	void SetBase(const json::Array& arr);
	void SetStop(const json::Dict& dict);
	void SetDistance(const json::Dict& dict);
	void SetBus(const json::Dict& dict);
	void SetMap(const json::Dict& dict, std::ostream& out);
	void SetRoutingSettings(const json::Dict& dict);

	json::Dict GetError(const int id);
	json::Array GetInfo(const json::Array& arr);
	json::Dict GetStop(const json::Dict& dict);
	json::Dict GetBus(const json::Dict& dict);
	json::Dict GetMap(const json::Dict& dict);
	json::Dict GetRoute(const json::Dict& dict);

private:
	TransportCatalogue::TransportCatalogue tc_;
	std::stringstream map_out_;
	TransportRouter::TransportRouter tr_;
};