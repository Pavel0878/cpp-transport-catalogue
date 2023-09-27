#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

#include <iostream>
#include <string_view>
#include <string>
#include <vector>

/*
class JsonReader {
public:
	//JsonReader(json::Document input)
	//	:input_(input){}
	JsonReader() = default;

	void Read(TransportCatalogue& tc, std::istream& is, std::ostream& out);

private:
	//json::Document input_;
};
*/

void Read(std::istream& is = std::cin, std::ostream& out = std::cout);