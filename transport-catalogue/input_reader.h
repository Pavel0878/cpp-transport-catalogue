#pragma once

#include "transport_catalogue.h"

#include <string>
#include <iostream>
#include <istream>
#include <string_view>
#include <unordered_map>
#include <map>

void Load(TransportCatalogue& tc, std::istream& is);