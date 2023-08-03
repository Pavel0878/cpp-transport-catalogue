#include "transport_catalogue.h"
#include "stat_reader.h"
#include "input_reader.h"

#include <iostream>

using namespace std;

int main() {
	TransportCatalogue tr;

		Load(tr, cin);
		LoadInfo(tr, cin, cout);
	//StatBusInfo(tr);
}