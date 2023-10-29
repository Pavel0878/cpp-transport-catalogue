#pragma once

#include "domain.h"

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

namespace TransportCatalogue {

	class TransportCatalogue {
	public:

		void AddStop(std::string_view name, double latitude, double longitude);
		void AddBus(std::string_view number, const std::vector<std::string_view>& stops, bool ring);
		void AddDistanceStop(std::string_view stop, std::string_view stop2, int distance);

		const Stop* FindStop(std::string_view name) const;
		const Bus* FindBus(std::string_view number) const;
		int FindDistance(std::string_view name, std::string_view name2, bool ring) const;
		std::optional<std::set<std::string_view>> GetStopInfo(std::string_view stop) const;
		BusInfo  GetBusInfo(std::string_view number) const;

		const std::deque<Stop>& GetStops() const;
		std::set<std::string_view> GetAllBus();


	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
		std::unordered_map<std::pair<Stop*, Stop*>, int, StopHash> distance_stops_;
	};
}  //namespace TransportCatalogue