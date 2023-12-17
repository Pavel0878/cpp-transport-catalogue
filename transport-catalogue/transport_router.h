#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include<string_view>
#include<string>
#include<optional>
#include<memory>
#include<unordered_map>

namespace TransportRouter {

	static  const double TO_METR_MIN = 1000.0 / 60.0;

	struct RoutingSettings {
		double bus_velocity = 0.;
		double bus_wait_time = 0.;
	};

	class TransportRouter {
	public:

		void SetRoutingSettings(const TransportCatalogue::TransportCatalogue& tc, RoutingSettings& routing_settings);
	const std::optional<graph::Router<double>::RouteInfo> BuildTransportRouter(const std::string from, const std::string to);
	void AddGrapnStops(const TransportCatalogue::TransportCatalogue& tc);
	void AddGraphEdge(const TransportCatalogue::TransportCatalogue& tc);

	const graph::DirectedWeightedGraph<double>& GetGraph() const;
	
	private:
		RoutingSettings routing_settings_;
		graph::DirectedWeightedGraph<double> graph_;
		std::unique_ptr<graph::Router<double>> router_ = nullptr;
		std::unordered_map<std::string_view, size_t> stops_id_;
	};

} // namespace TransportRouter