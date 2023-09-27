#pragma once

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

#include "svg.h"
#include "geo.h"
#include "transport_catalogue.h"
#include "json.h"
#include "json_reader.h"

#include <iostream>
#include <string_view>
#include <set>
#include <vector>

void SvgRead(TransportCatalogue& tc, const json::Dict& dict, std::ostream& out);