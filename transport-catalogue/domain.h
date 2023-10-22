#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace TransportCatalogue {

	struct Stop {
		std::string stop_name;
		geo::Coordinates coordinates;
		//double latitude; //широта
		//double longitude; //долгота
	};

	using StopPtr = const Stop*;

	struct Bus {
		std::string bus_number;
		std::vector<Stop*> bus_stops;
		bool bus_ring;
	};

	struct BusInfo {
		std::string_view number_bus;
		size_t stops_on_route = 0;
		size_t unique_stops = 0;
		double route_length = 0.0;
		int actual_length = 0;
		double real_number = 0.0;
	};


	struct StopHash {
		size_t operator() (const std::pair<Stop*, Stop*>& stop) const {
			size_t st1 = hasher_(stop.first);
			size_t st2 = hasher_(stop.second);
			return st1 * 1000 + st2;
		}

	private:
		std::hash<const void*> hasher_;
	};
} // namespace TransportCatalogue