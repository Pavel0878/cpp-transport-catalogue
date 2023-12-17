#include "transport_router.h"

#include <iostream>

namespace TransportRouter {

	void TransportRouter::SetRoutingSettings(const TransportCatalogue::TransportCatalogue& tc, RoutingSettings& routing_settings) {
		routing_settings_ = std::move(routing_settings);
		AddGrapnStops(tc);
		AddGraphEdge(tc);
		router_ = std::make_unique<graph::Router<double>>(graph_);
	}

	const std::optional<graph::Router<double>::RouteInfo> TransportRouter::BuildTransportRouter(
		const std::string from, const std::string to) {
		return router_->BuildRoute(stops_id_.at(from), stops_id_.at(to));
	}
	void TransportRouter::AddGrapnStops(const TransportCatalogue::TransportCatalogue& tc) {
		const auto& stops = tc.GetStops();
		graph_ = graph::DirectedWeightedGraph<double>(stops.size() * 2);
		graph::VertexId vertex_id = 0;
		size_t stop_count = 0;
		for (const auto& stop : stops) {
			stops_id_[stop.stop_name] = vertex_id;
			graph_.AddEdge({ stop.stop_name, stop_count, vertex_id, ++vertex_id, routing_settings_.bus_wait_time });
			++vertex_id;
		}
	}
	void TransportRouter::AddGraphEdge(const TransportCatalogue::TransportCatalogue& tc) {
		for (const auto& bus : tc.GetAllBus()) {
			const auto& bus_info = tc.FindBus(bus);
			const auto& bus_stops = bus_info->bus_stops;
			const auto& ring = tc.FindBus(bus)->bus_ring;
			size_t bus_stops_count = bus_stops.size();
			size_t bus_stops_count2 = bus_stops_count;

			if (bus_stops_count <= 1) { continue; }

			for (size_t i = 0; i +1 < bus_stops_count; ++i) {
				auto first_stop = bus_stops[i]->stop_name;
				auto from = stops_id_.at(first_stop);
				int distance = 0;
				int reverse = 0;

				for (size_t j = i + 1; j < bus_stops_count2; ++j) {
					
					auto second_stop = bus_stops[j - 1]->stop_name;
					auto second_stop2 = bus_stops[j]->stop_name;
 					auto to = stops_id_.at(second_stop2);
					distance += tc.FindDistance(second_stop, second_stop2, true);
					reverse += tc.FindDistance(second_stop2, second_stop, true);
					graph_.AddEdge({ bus_info->bus_number, j-i, from + 1, to,
						static_cast<double>(distance) / (routing_settings_.bus_velocity * TO_METR_MIN)});

					if (!ring) {
						graph_.AddEdge({ bus_info->bus_number, j - i, to + 1, from,
							static_cast<double>(reverse) / (routing_settings_.bus_velocity * TO_METR_MIN) });
					}
				}
			}
		}
	}

	const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
		return graph_;
	}

} // namespace TransportRouter