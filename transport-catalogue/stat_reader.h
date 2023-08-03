#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <istream>
#include <string>
#include <unordered_set>
#include <string_view>
#include <stdexcept>

void LoadInfo(TransportCatalogue& tr, std::istream& is, std::ostream& out);