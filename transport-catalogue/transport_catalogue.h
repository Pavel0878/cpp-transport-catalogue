#pragma once

#include "geo.h"

#include <deque>
#include <string>
#include <vector>
#include <string_view>
#include <unordered_map>
#include <stdexcept>
#include <optional>
#include <unordered_set>
#include <set>
#include <algorithm>

class TransportCatalogue {
public:

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


	void AddStop(std::string_view name, double longitude, double latitude);

	const Stop* FindStop(std::string_view name) const;

	void AddBus(std::string_view number, const std::vector<std::string_view>& stops, bool ring);

	const Bus* FindBus(std::string_view number) const;

	void AddDistanceStop(std::string_view stop, std::string_view stop2, int distance);

	int FindDistance(std::string_view name, std::string_view name2, bool ring);

	std::optional<std::set<std::string_view>> GetStopInfo(std::string_view stop) const;

	BusInfo  GetBusInfo(std::string_view number);

	const std::set<std::string_view> GetBuses() const;

//	const std::set<std::string_view> GetStops() const;

private:
	std::deque<Stop> stops_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
	std::unordered_map<std::string_view, Bus*> busname_to_bus_;
	std::unordered_map<std::pair<Stop*, Stop*>, int, StopHash> distance_stops_;
};