#include "map_renderer.h"
#include <set>


namespace renderer {

std::map<std::string_view, std::tuple<std::vector<std::pair<std::string, svg::Point>>, bool>> MapRenderer::TranformCoordinates(const std::unordered_map<std::string_view, transport_catalogue::Bus*>& routes) const{
    std::vector<geo::Coordinates> merged_coordinates;
    for (const auto& [name, bus] : routes){
        for (const auto stop : bus->stops){
            merged_coordinates.push_back(stop->coordinates);
        }
    }

    SphereProjector projector_(merged_coordinates.begin(), merged_coordinates.end(), settings_.width, settings_.height, settings_.padding);

    std::map<std::string_view, std::tuple<std::vector<std::pair<std::string, svg::Point>>, bool>> result;

    for (const auto& [name, bus] : routes){
        std::vector<std::pair<std::string, svg::Point>> polyline;
        if (bus->is_circular){
            for (const auto& stop : bus->stops){
                    polyline.push_back({stop->name, projector_(stop->coordinates)});            
            }
        }
        else {
            for (size_t i = 0; i < bus->stops.size(); ++i){
                polyline.push_back({bus->stops[i]->name, projector_(bus->stops[i]->coordinates)});
            }
            for (size_t i = bus->stops.size() - 1; i > 0; --i){
                polyline.push_back({bus->stops[i]->name, projector_(bus->stops[i - 1]->coordinates)});
            }
        }
        std::tuple<std::vector<std::pair<std::string, svg::Point>>, bool> tuple{polyline, bus->is_circular};
        result.emplace(name, tuple);
    }

    return result;
}
svg::Text CreateText(const std::string& name, const RenderingSettings& settings_, const svg::Point point, int i){
    svg::Text result;
    result.SetData(name)
        .SetPosition(point)
        .SetOffset({settings_.bus_label_offset.dx, settings_.bus_label_offset.dy})
        .SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetFillColor(settings_.color_palette[i]);
        return result;
}

svg::Text CreateUnderLayerText(const std::string& name, const RenderingSettings& settings_, const svg::Point point){
    svg::Text result;
    result.SetData(name)
        .SetPosition(point)
        .SetOffset({settings_.bus_label_offset.dx, settings_.bus_label_offset.dy})
        .SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        return result;
}

svg::Document MapRenderer::RenderRoutes(const std::map<std::string_view, std::tuple<std::vector<std::pair<std::string, svg::Point>>, bool>>& routes) const {
    svg::Document document;
    size_t i = 0;
    for (const auto& [bus, route] : routes){
        std::vector<std::pair<std::string, svg::Point>> stops = std::get<std::vector<std::pair<std::string, svg::Point>>>(route);
        svg::Polyline line;
        for (const auto& point : stops){
            line.AddPoint(point.second);
        }        
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeWidth(settings_.line_width)
            .SetFillColor(svg::Color{"none"})
            .SetStrokeColor(settings_.color_palette[i])
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            
        document.Add(line);
        if (i == settings_.color_palette.size() - 1){
            i = 0;
        } else {
            ++i;
        }
    }
    i = 0;
    for (const auto& [bus, route] : routes){
        std::vector<std::pair<std::string, svg::Point>> stops = std::get<std::vector<std::pair<std::string, svg::Point>>>(route);
        if (std::get<bool>(route) == true){
            svg::Text name = CreateText(static_cast<std::string>(bus), settings_, stops[0].second, i);
            svg::Text undername = CreateUnderLayerText(static_cast<std::string>(bus), settings_, stops[0].second);
            document.Add(undername);
            document.Add(name);
        }
        else {
            size_t index_last = ((stops.size() + 1) / 2) - 1;
            svg::Text name_first = CreateText(static_cast<std::string>(bus), settings_, stops[0].second, i);
            svg::Text undername_first = CreateUnderLayerText(static_cast<std::string>(bus), settings_, stops[0].second);
            document.Add(undername_first);
            document.Add(name_first);
            if (stops[index_last].first != stops[0].first){
                svg::Text name_last = CreateText(static_cast<std::string>(bus), settings_, stops[index_last].second, i);
                svg::Text undername_last = CreateUnderLayerText(static_cast<std::string>(bus), settings_, stops[index_last].second);
                document.Add(undername_last);
                document.Add(name_last);
            }
        }
        if (i == settings_.color_palette.size() - 1){
            i = 0;
        } else {
            ++i;
        }
    }

    std::vector<std::pair<std::string, svg::Point>> joined_stops;
    for (const auto& [bus, route] : routes){
        std::vector<std::pair<std::string, svg::Point>> r = std::get<0>(route);
        std::copy(r.begin(), r.end(), std::back_inserter(joined_stops));
    }
   auto comparator = [](const std::pair<std::string, svg::Point>& left, const std::pair<std::string, svg::Point>& right){ return left.first < right.first; };
    std::set<std::pair<std::string, svg::Point>, decltype(comparator)> joined_stops_set(comparator);
    for (auto it = joined_stops.begin(); it != joined_stops.end(); ++it){
        joined_stops_set.insert(*it);
    }
    for (const auto& stop : joined_stops_set){
        svg::Circle c;
        c.SetCenter({stop.second.x, stop.second.y})
            .SetRadius(settings_.stop_radius)
            .SetFillColor(svg::Color("white"));
            document.Add(c);
    }

    for (const auto& stop : joined_stops_set){
        svg::Text name;
        name.SetData(stop.first)
        .SetPosition({stop.second.x , stop.second.y})
        .SetOffset({settings_.stop_label_offset.dx, settings_.stop_label_offset.dy})
        .SetFontSize(settings_.stop_label_font_size)
        .SetFontFamily("Verdana")
        .SetFillColor(svg::Color("black"));

        svg::Text undername;
        undername.SetData(stop.first)
        .SetPosition({stop.second.x , stop.second.y})
        .SetOffset({settings_.stop_label_offset.dx, settings_.stop_label_offset.dy})
        .SetFontSize(settings_.stop_label_font_size)
        .SetFontFamily("Verdana")
        .SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        document.Add(undername);
        document.Add(name);
    }
    
    return document;
}

}