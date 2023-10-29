#include "transport_catalogue.h"

#include <iostream>
#include <iomanip>

using namespace std;

namespace TransportCatalogue {

	void TransportCatalogue::AddStop(string_view name, double latitude, double longitude) {
		//	if(latitude < -90.0 && 90.0 < latitude) { throw invalid_argument("широта не верна"s); }
		//	if (longitude < -180.0 && 180.0 < longitude) { throw invalid_argument("долгота не верна"s); }
		Stop st = { move(string{name}), latitude, longitude };
		Stop& stop = stops_.emplace_back(st);
		stopname_to_stop_[stop.stop_name] = &stop;
	}

	void TransportCatalogue::AddBus(string_view number, const vector<string_view>& stops, bool ring) {
		vector<Stop*> tmp;
		for (auto& stop : stops) {
			auto stop_tmp = stopname_to_stop_.at(stop);
			tmp.push_back(stop_tmp);
		}
		Bus tp = { move(string{number}), tmp, ring };
		Bus& bus = buses_.emplace_back(move(tp));
		busname_to_bus_[bus.bus_number] = &bus;
	}

	void TransportCatalogue::AddDistanceStop(string_view name, string_view name2, int distance) {
		auto stop1 = stopname_to_stop_.at(name);
		auto stop2 = stopname_to_stop_.at(name2);
		distance_stops_[{stop1, stop2}] = distance;
	}

	const Stop* TransportCatalogue::FindStop(string_view name) const {
		return (stopname_to_stop_.find(name) != stopname_to_stop_.end()) ? stopname_to_stop_.at(name) : nullptr;
	}

	const Bus* TransportCatalogue::FindBus(string_view number) const {
		return (busname_to_bus_.find(number) != busname_to_bus_.end()) ? busname_to_bus_.at(number) : nullptr;
	}

	int TransportCatalogue::FindDistance(string_view name, string_view name2, bool ring) const {
		Stop* st = stopname_to_stop_.at(name);
		Stop* st2 = stopname_to_stop_.at(name2);
		int tmp1 = distance_stops_.count({ st,st2 }) ? distance_stops_.at({ st, st2 }) : distance_stops_.at({ st2, st });

		if (ring == true) {
			return tmp1;
		}
		int tmp = distance_stops_.count({ st2,st }) ? distance_stops_.at({ st2, st }) : distance_stops_.at({ st, st2 });
		return tmp1 + tmp;
	}

	optional<set<string_view>> TransportCatalogue::GetStopInfo(string_view stop) const {
		const auto& tmp = FindStop(stop);
		if (tmp == nullptr) { return nullopt; }

		set<string_view> buses;

		for (const auto& bus : busname_to_bus_) {
			auto tp = find(bus.second->bus_stops.begin(), bus.second->bus_stops.end(), tmp);
			if (tp != bus.second->bus_stops.end()) { buses.insert(bus.second->bus_number); }
		}
		return buses;
	}

	BusInfo TransportCatalogue::GetBusInfo(string_view number) const {
		const auto bus = FindBus(number);
		BusInfo result;
		result.number_bus = number;
		if (!bus) { return result; }

		unordered_set<string_view> stops;

		for (auto& stop : bus->bus_stops) {
			stops.insert(stop->stop_name);
		}

		result.unique_stops = stops.size();

		for (auto stop1 = bus->bus_stops.begin(), stop2 = stop1 + 1;
			stop2 < bus->bus_stops.end(); ++stop1, ++stop2) {
			result.route_length += ComputeDistance((*stop1)->coordinates, (*stop2)->coordinates);

			result.actual_length += FindDistance((*stop1)->stop_name, (*stop2)->stop_name, bus->bus_ring);
		}
		result.route_length = bus->bus_ring == 1 ? result.route_length : result.route_length * 2;
		result.stops_on_route = (bus->bus_ring == 1 ? bus->bus_stops.size() : bus->bus_stops.size() * 2 - 1);
		result.real_number = result.actual_length / result.route_length;

		return result;
	}

	set<string_view> TransportCatalogue::GetAllBus() {
		set<string_view> name_bus;
		for (auto [name, bus] : busname_to_bus_) {
			name_bus.insert(name);
		}
		return name_bus;
	}

	const deque<Stop>& TransportCatalogue::GetStops() const {
		return stops_;
	}

}  //namespace TransportCatalogue