#pragma once

#include "svg.h"
#include "geo.h"
#include "domain.h"

#include <iostream>
#include <string_view>
#include <set>
#include <vector>
#include <deque>
#include <unordered_map>

namespace renderer {
    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding);

		// Проецирует широту и долготу в координаты внутри SVG-изображения
		svg::Point operator()(geo::Coordinates coords) const {
			return {
				(coords.lng - min_lon_) * zoom_coeff_ + padding_,
				(max_lat_ - coords.lat) * zoom_coeff_ + padding_
			};
		}

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

	struct Map {
		double width = 0.0;
		double height = 0.0;
		double padding = 0.0;
		double line_width = 0.0;
		double stop_radius = 0.0;
		unsigned int bus_label_font_size = 0;
		svg::Point bus_label_offset {0.0, 0.0};
		unsigned int stop_label_font_size = 0;
		svg::Point stop_label_offset {0.0, 0.0};
		svg::Color underlayer_color;
		double underlayer_width = 0.0;
		std::vector <svg::Color> color_palette;
	};

	class MapRenderer {
	public:
		MapRenderer(Map& map)
			:map_(map){}

		const SphereProjector& FillProjector(std::vector<geo::Coordinates> geo_coords);

		void SetRoute(const std::vector<TransportCatalogue::Bus>& all_buses
			, const std::deque<TransportCatalogue::Stop>& all_stops);
		
		void SetPolylineRout(std::vector<svg::Point> coordinates_stops, bool bus_ring);
		void SetNameRoute(svg::Point bus, std::string bus_name);
		void SetCircleStop(svg::Point stop);
		void SetNameStop(svg::Point stop,std::string stop_name);

		void GetDocument(std::ostream& out);

	private:
		Map map_;
		size_t number_color_ = 0;
		svg::Document doc_;
		std::vector<svg::Text> name_route_;
		std::vector<svg::Circle> circle_stop_;
		std::vector<svg::Text> name_stop_;
	};
} // namespece renderer