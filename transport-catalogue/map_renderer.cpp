#include "map_renderer.h"

#include <algorithm>
#include <cstdlib>
#include <optional>

namespace renderer {
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector::SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }
        
        void MapRenderer::SetRoute(const std::vector<TransportCatalogue::Bus>& all_buses
            , const std::deque<TransportCatalogue::Stop>& all_stops){

            // Точки, подлежащие проецированию
            std::vector<geo::Coordinates> geo_coords;
            std::set<std::string_view> stop_all_bus;
            for (const auto bus : all_buses) {
                if (bus.bus_stops.empty()) { continue; }
                for (auto stop : bus.bus_stops) {
                    geo_coords.push_back(stop->coordinates);
                    stop_all_bus.insert(stop->stop_name);
                }
            }

             // Создаём проектор сферических координат на карту
            const SphereProjector proj{
                geo_coords.begin(), geo_coords.end()
                , map_.width//WIDTH
                , map_.height//HEIGHT
                , map_.padding //PADDING
            };

           for (const auto bus : all_buses) {
                bool bus_ring = bus.bus_ring;
                std::vector<svg::Point> coordinates_stops;
               for (auto sto : bus.bus_stops) {
                   coordinates_stops.push_back(proj(sto->coordinates));
               }
               SetPolylineRout(coordinates_stops, bus_ring);
               TransportCatalogue::Stop* st_begin = *bus.bus_stops.begin();
               TransportCatalogue::Stop* st_back = bus.bus_stops.back();
                SetNameRoute(proj(st_begin->coordinates), bus.bus_number);
                if (!bus_ring && st_begin != st_back) {
                    SetNameRoute(proj(st_back->coordinates), bus.bus_number);
                }
                ++number_color_;
                if (number_color_ == map_.color_palette.size()) { number_color_ = 0; }
            }


            for (const auto stop : stop_all_bus) {
                for(auto st : all_stops){
                    if (stop == st.stop_name) {
                        SetCircleStop(proj(st.coordinates));
                        SetNameStop(proj(st.coordinates), st.stop_name);
                    }
                }
            }
        }
        


        void MapRenderer::SetPolylineRout(std::vector<svg::Point> coordinates_stops, bool bus_ring) {
        svg::Polyline polyline;
        std::vector <svg::Point> stops_pop;

        for (auto stop : coordinates_stops) {
            polyline.AddPoint(stop);
            stops_pop.emplace_back(stop);
        }

        if (!bus_ring ) {
            stops_pop.pop_back();
            for (auto i = stops_pop.size(); i > 0; i--) {
                polyline.AddPoint(stops_pop[i - 1]);
            }
        }
        polyline.SetFillColor("none");
        polyline.SetStrokeColor(map_.color_palette[number_color_]);
        polyline.SetStrokeWidth(map_.line_width);
        polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        doc_.Add(polyline);
        }

        void MapRenderer::SetNameRoute(svg::Point bus, std::string bus_name) {
            svg::Text background;
            svg::Text name_bus;
            background.SetPosition(bus);
            name_bus.SetPosition(bus);
            background.SetOffset(map_.bus_label_offset);
            name_bus.SetOffset(map_.bus_label_offset);
            background.SetFontSize(map_.bus_label_font_size);
            name_bus.SetFontSize(map_.bus_label_font_size);
            background.SetFontFamily("Verdana");
            name_bus.SetFontFamily("Verdana");
            background.SetFontWeight("bold");
            name_bus.SetFontWeight("bold");
            background.SetData(bus_name);
            name_bus.SetData(bus_name);
            background.SetFillColor(map_.underlayer_color);
            name_bus.SetFillColor(map_.color_palette[number_color_]);
            background.SetStrokeColor(map_.underlayer_color);
            background.SetStrokeWidth(map_.underlayer_width);
            background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            name_route_.push_back(background);
            name_route_.push_back(name_bus);
        }

        void MapRenderer::SetCircleStop(svg::Point stop) {
            svg::Circle stop_circle;
                stop_circle.SetCenter(stop);
                stop_circle.SetRadius(map_.stop_radius);
                stop_circle.SetFillColor("white");
                circle_stop_.push_back(stop_circle);
        }

        void MapRenderer::SetNameStop(svg::Point stop, std::string stop_name) {
                svg::Text background;
                svg::Text name_stop;
                background.SetPosition(stop);
                name_stop.SetPosition(stop);
                background.SetOffset(map_.stop_label_offset);
                name_stop.SetOffset(map_.stop_label_offset);
                background.SetFontSize(map_.stop_label_font_size);
                name_stop.SetFontSize(map_.stop_label_font_size);
                background.SetFontFamily("Verdana");
                name_stop.SetFontFamily("Verdana");
                background.SetData(stop_name);
                name_stop.SetData(stop_name);
                background.SetFillColor(map_.underlayer_color);
                name_stop.SetFillColor("black");
                background.SetStrokeColor(map_.underlayer_color);
                background.SetStrokeWidth(map_.underlayer_width);
                background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                name_stop_.push_back(background);
                name_stop_.push_back(name_stop);
        }

        void MapRenderer::GetDocument(std::ostream& out) {
            for (auto name : name_route_) {
                doc_.Add(name);
            }
            for (auto circle : circle_stop_) {
                doc_.Add(circle);
            }
            for (auto stop : name_stop_) {
                doc_.Add(stop);
            }
             doc_.Render(out);
        }

} // namespace renderer