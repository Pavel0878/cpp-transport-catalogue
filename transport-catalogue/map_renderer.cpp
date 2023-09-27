#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

inline const double EPSILON = 1e-6;
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
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
/*
svg::Color ConvertColor(const json::Array color) {
    svg::Color result;
        if (color.size() == 3) {
            uint8_t r = static_cast<uint8_t>(color[0].AsDouble());
            uint8_t g = static_cast<uint8_t>(color[1].AsDouble());
            uint8_t b = static_cast<uint8_t>(color[2].AsDouble());
            result = svg::Rgb{r, g, b};
        }
        else {
            uint8_t r = static_cast<uint8_t>(color[0].AsDouble());
            uint8_t g = static_cast<uint8_t>(color[1].AsDouble());
            uint8_t b = static_cast<uint8_t>(color[2].AsDouble());
            double opacity = color[3].AsDouble();
            result = svg::Rgba{r, g, b, opacity};
        }
    return result;
}
*/
svg::Color ConvertColor(const json::Node colors) {
    if (colors.IsArray()) {
        auto color = colors.AsArray();
        if (color.size() == 3) {
            uint8_t r = static_cast<uint8_t>(color[0].AsDouble());
            uint8_t g = static_cast<uint8_t>(color[1].AsDouble());
            uint8_t b = static_cast<uint8_t>(color[2].AsDouble());
            return svg::Rgb{ r, g, b };
        }
        else {
            uint8_t r = static_cast<uint8_t>(color[0].AsDouble());
            uint8_t g = static_cast<uint8_t>(color[1].AsDouble());
            uint8_t b = static_cast<uint8_t>(color[2].AsDouble());
            double opacity = color[3].AsDouble();
            return svg::Rgba{ r, g, b, opacity };
        }
    }
    return colors.AsString();
}

void PolylineRoutes(svg::Document& doc, TransportCatalogue& tc, const json::Dict& dict, const SphereProjector& proj) {
    size_t number_color = 0;
    for (const auto bus : tc.GetBuses()) {
        auto find_bus =tc.FindBus(bus);
        if (find_bus->bus_stops.empty()) { continue; }

       // std::cout << dict.at("width").AsInt() << std::endl;
        svg::Polyline polyline;
        std::vector <svg::Point> stops;
        for (auto stop : tc.FindBus(bus)->bus_stops) {
            polyline.AddPoint(proj(stop->coordinates));
            stops.emplace_back(proj(stop->coordinates));
        }
        if (!find_bus->bus_ring) {
            stops.pop_back();
            for (auto i = stops.size(); i > 0; i--) {
                polyline.AddPoint(stops[i-1]);
            }
        }
        polyline.SetFillColor("none");
        auto colors = dict.at("color_palette").AsArray();
     //   if (colors[number_color].IsString()) {
      //      polyline.SetStrokeColor(colors[number_color].AsString());
     //   }
     //   if (colors[number_color].IsArray()) {
     //       polyline.SetStrokeColor(ConvertColor(colors[number_color].AsArray()));
     //   }
        polyline.SetStrokeColor(ConvertColor(colors[number_color]));
        polyline.SetStrokeWidth(dict.at("line_width").AsDouble());
        polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        doc.Add(polyline);
        ++number_color;
        if (number_color == colors.size()) { number_color = 0; }
    }
}

void RouteNames(svg::Document& doc, TransportCatalogue& tc, const json::Dict& dict, const SphereProjector& proj) {
    size_t number_color = 0;
    for (const auto bus : tc.GetBuses()) {
        auto find_bus = tc.FindBus(bus);
        if (find_bus->bus_stops.empty()) { continue; }
        svg::Text background_bus;
        svg::Text title_bus;
        background_bus.SetFillColor(ConvertColor(dict.at("underlayer_color")));
        background_bus.SetStrokeColor(ConvertColor(dict.at("underlayer_color")));
        auto colors = dict.at("color_palette").AsArray();
   //     if (colors[number_color].IsString()) {
   //         title_bus.SetFillColor(colors[number_color].AsString());
   //     }
   //     if (colors[number_color].IsArray()) {
   //         title_bus.SetFillColor(ConvertColor(colors[number_color].AsArray()));
  //      }
        title_bus.SetFillColor(ConvertColor(colors[number_color]));
        background_bus.SetStrokeWidth(dict.at("underlayer_width").AsDouble());
        background_bus.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        background_bus.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        auto stop = *find_bus->bus_stops.begin();
        background_bus.SetPosition(proj(stop->coordinates));
        title_bus.SetPosition(proj(stop->coordinates));
        auto bus_offset = dict.at("bus_label_offset").AsArray();
        svg::Point offset(bus_offset[0].AsDouble(), bus_offset[1].AsDouble());
        background_bus.SetOffset(offset);
        title_bus.SetOffset(offset);
        background_bus.SetFontSize(dict.at("bus_label_font_size").AsInt());
        title_bus.SetFontSize(dict.at("bus_label_font_size").AsInt());
        background_bus.SetFontFamily("Verdana");
        title_bus.SetFontFamily("Verdana");
        background_bus.SetFontWeight("bold");
        title_bus.SetFontWeight("bold");
        background_bus.SetData(std::string(bus));
        title_bus.SetData(std::string(bus));
        doc.Add(background_bus);
        doc.Add(title_bus);
        if (!find_bus->bus_ring && (*find_bus->bus_stops.begin())!= (find_bus->bus_stops.back())) {
            svg::Text background_bus;
            svg::Text title_bus;
            background_bus.SetFillColor(ConvertColor(dict.at("underlayer_color")));
            background_bus.SetStrokeColor(ConvertColor(dict.at("underlayer_color")));
            auto colors = dict.at("color_palette").AsArray();
         //   if (colors[number_color].IsString()) {
         //       title_bus.SetFillColor(colors[number_color].AsString());
        //    }
        //    if (colors[number_color].IsArray()) {
         //       title_bus.SetFillColor(ConvertColor(colors[number_color].AsArray()));
        //    }
            title_bus.SetFillColor(ConvertColor(colors[number_color]));
            background_bus.SetStrokeWidth(dict.at("underlayer_width").AsDouble());
            background_bus.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            background_bus.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            auto stop = find_bus->bus_stops.back();
            background_bus.SetPosition(proj(stop->coordinates));
            title_bus.SetPosition(proj(stop->coordinates));
            auto bus_offset = dict.at("bus_label_offset").AsArray();
            svg::Point offset(bus_offset[0].AsDouble(), bus_offset[1].AsDouble());
            background_bus.SetOffset(offset);
            title_bus.SetOffset(offset);
            background_bus.SetFontSize(dict.at("bus_label_font_size").AsInt());
            title_bus.SetFontSize(dict.at("bus_label_font_size").AsInt());
            background_bus.SetFontFamily("Verdana");
            title_bus.SetFontFamily("Verdana");
            background_bus.SetFontWeight("bold");
            title_bus.SetFontWeight("bold");
            background_bus.SetData(std::string(bus));
            title_bus.SetData(std::string(bus));
            doc.Add(background_bus);
            doc.Add(title_bus);
        }
        ++number_color;
        if (number_color == colors.size()) { number_color = 0; }
    }
}

void CircleStop(svg::Document& doc, TransportCatalogue& tc,
    std::set<std::string_view>& stops, const json::Dict& dict, const SphereProjector& proj) {
    for (const auto stop : stops) {
        svg::Circle circle_stop;
        circle_stop.SetCenter(proj(tc.FindStop(stop)->coordinates));
        circle_stop.SetRadius(dict.at("stop_radius").AsDouble());
        circle_stop.SetFillColor("white");
        doc.Add(circle_stop);
    }
}
void NameStop(svg::Document& doc, TransportCatalogue& tc,
    std::set<std::string_view>& stops, const json::Dict& dict, const SphereProjector& proj) {
    for (const auto stop : stops) {
        svg::Text background_stop;
        svg::Text title_stop;
        auto tp = dict.at("underlayer_color");
        background_stop.SetFillColor(ConvertColor(dict.at("underlayer_color")));
        background_stop.SetStrokeColor(ConvertColor(dict.at("underlayer_color")));
        title_stop.SetFillColor("black");
        background_stop.SetStrokeWidth(dict.at("underlayer_width").AsDouble());
        background_stop.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        background_stop.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        background_stop.SetPosition(proj(tc.FindStop(stop)->coordinates));
        title_stop.SetPosition(proj(tc.FindStop(stop)->coordinates));
        auto stop_offset = dict.at("stop_label_offset").AsArray();
        svg::Point offset(stop_offset[0].AsDouble(), stop_offset[1].AsDouble());
        background_stop.SetOffset(offset);
        title_stop.SetOffset(offset);
        background_stop.SetFontSize(dict.at("stop_label_font_size").AsDouble());
        title_stop.SetFontSize(dict.at("stop_label_font_size").AsDouble());
        background_stop.SetFontFamily("Verdana");
        title_stop.SetFontFamily("Verdana");
        background_stop.SetData(std::string(stop));
        title_stop.SetData(std::string(stop));
        doc.Add(background_stop);
        doc.Add(title_stop);
    }
}

void SvgRead(TransportCatalogue& tc, const json::Dict& dict, std::ostream& out) {
    std::set<std::string_view> stops; // Остановки в маршрутах
    std::vector<geo::Coordinates> geo_coords; // Точки, подлежащие проецированию
    for (const auto bus : tc.GetBuses()) {
        auto find_bus = tc.FindBus(bus);
        if (find_bus->bus_stops.empty()) { continue; }
        for (auto stop : find_bus->bus_stops) {
         //   if (!find_bus->bus_stops.empty()) {
                stops.insert(stop->stop_name);
           // }
          //  geo_coords.push_back(stop->coordinates);
        }
    }

    for (auto buses : tc.GetBuses()) {
        for (auto stop : tc.FindBus(buses)->bus_stops) {
            geo_coords.push_back(stop->coordinates);
        }

    }

    // Создаём проектор сферических координат на карту
    const SphereProjector proj{
        geo_coords.begin(), geo_coords.end()
        , dict.at("width").AsDouble()//WIDTH
        , dict.at("height").AsDouble()//HEIGHT
        , dict.at("padding").AsDouble() //PADDING
    };
   
    svg::Document doc;
    PolylineRoutes(doc, tc, dict, proj);
    RouteNames(doc, tc, dict, proj);
    CircleStop(doc, tc, stops, dict, proj);
    NameStop(doc, tc, stops, dict, proj);
    doc.Render(out);
}

