#include "stat_reader.h"


using namespace std;

void PrintBusInfo(TransportCatalogue& tr, string_view bus) {
	auto tmp = tr.GetBusInfo(bus);
	if (!tmp.unique_stops) {
		cout << "Bus "s << bus << ": not found" << endl;
	}
	else {
		cout << "Bus "s << tmp.number_bus << ": "s << tmp.stops_on_route << " stops on route, "s
			<< tmp.unique_stops << " unique stops, "s << tmp.actual_length << " route length, "s 
			<< tmp.real_number << " curvature"s << endl;
	}
}

void PrintStopInfo(TransportCatalogue& tc, string_view stop) {

	optional<set<string_view>> tmp = tc.GetStopInfo(stop);
      if (tmp == nullopt) {
		cout << "Stop "sv << string(stop) << ": not found"sv << endl;
	}else if (tmp->empty()) {
		cout << "Stop "sv << string(stop) << ": no buses "sv << endl;
	}else{
			cout << "Stop "sv << string(stop) << ": buses"sv;
			for (auto& str : *tmp) {
				cout << ' ' << string(str);
			}
			cout << endl;
	     }
}

void LoadInfo(TransportCatalogue& tc) {
    int num;
    string s;
    cin >> num;
    int i = 0;
    getline(cin, s);
    while (i < num) {
        ++i;
        getline(cin, s);
        size_t pos = s.find(' ');
        string com = s.substr(0, pos);
		string str = s.substr(pos + 1);
        if (com == "Stop"s) { 
			PrintStopInfo(tc, str);
		}

        if (com == "Bus"s) {
			PrintBusInfo(tc, str);
        }
    }
}