#pragma once

#include "json.h"
#include "map_renderer.h"



json::Document GetQuery(std::istream& input);

renderer::RenderingSettings GetRenderingSettings(const json::Document& document);


/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */