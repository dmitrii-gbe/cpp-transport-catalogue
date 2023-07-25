#include "json_reader.h"



json::Document GetQuery(std::istream& input){
    json::Document queries = json::Load(input);
    return queries;
}

renderer::RenderingSettings GetRenderingSettings(const json::Document& document){
    json::Dict settings = document.GetRoot().AsMap().at("render_settings").AsMap();
    renderer::RenderingSettings result;
    result.width = settings.at("width").AsDouble();
    result.height = settings.at("height").AsDouble();
    result.padding = settings.at("padding").AsDouble();
    result.line_width = settings.at("line_width").AsDouble();
    result.stop_radius = settings.at("stop_radius").AsDouble();
    result.bus_label_font_size = settings.at("bus_label_font_size").AsInt();
    result.underlayer_width = settings.at("underlayer_width").AsDouble();
    
    json::Array bus_label_offset_arr = settings.at("bus_label_offset").AsArray();
    renderer::LabelOffset bus_label_offset = {bus_label_offset_arr[0].AsDouble(), bus_label_offset_arr[1].AsDouble()};
    result.bus_label_offset = bus_label_offset;
    
    result.stop_label_font_size = settings.at("stop_label_font_size").AsInt();

    json::Array stop_label_offset_arr = settings.at("stop_label_offset").AsArray();
    renderer::LabelOffset stop_label_offset = {stop_label_offset_arr[0].AsDouble(), stop_label_offset_arr[1].AsDouble()};
    result.stop_label_offset = stop_label_offset;
    if (settings.at("underlayer_color").IsArray()){
        json::Array ar = settings.at("underlayer_color").AsArray();
        if (ar.size() == 3){
            svg::Rgb rgb_color{static_cast<uint8_t>(ar[0].AsInt()), static_cast<uint8_t>(ar[1].AsInt()), static_cast<uint8_t>(ar[2].AsInt())};
            result.underlayer_color = rgb_color; 
        }
        if (ar.size() == 4) {
            svg::Rgba rgba_color{static_cast<uint8_t>(ar[0].AsInt()), static_cast<uint8_t>(ar[1].AsInt()), static_cast<uint8_t>(ar[2].AsInt()), ar[3].AsDouble()};
            result.underlayer_color = rgba_color; 
        }
    }
    if (settings.at("underlayer_color").IsString()) {
        result.underlayer_color = svg::Color(settings.at("underlayer_color").AsString());
    }

    json::Array ar = settings.at("color_palette").AsArray();
    for (const auto& item : ar){
        if (item.IsString()){
            result.color_palette.push_back(svg::Color(item.AsString()));
        }
        if (item.IsArray()){
            json::Array ar = item.AsArray();
            if (ar.size() == 3){
                svg::Rgb rgb_color{static_cast<uint8_t>(ar[0].AsInt()), static_cast<uint8_t>(ar[1].AsInt()), static_cast<uint8_t>(ar[2].AsInt())};
                result.color_palette.push_back(svg::Color(rgb_color));
            }
            if (ar.size() == 4) {
                svg::Rgba rgba_color{static_cast<uint8_t>(ar[0].AsInt()), static_cast<uint8_t>(ar[1].AsInt()), static_cast<uint8_t>(ar[2].AsInt()), ar[3].AsDouble()};
                result.color_palette.push_back(svg::Color(rgba_color));
            }
        }
    }
    return result;
}

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */