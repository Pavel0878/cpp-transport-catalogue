#include "stat_reader.h"

using namespace std;

void PrintBusInfo(TransportCatalogue& tr, string_view bus, ostream& out) {
	auto tmp = tr.GetBusInfo(bus);
	if (!tmp.unique_stops) {
		out << "Bus "s << bus << ": not found" << endl;
	}
	else {
		out << "Bus "s << tmp.number_bus << ": "s << tmp.stops_on_route << " stops on route, "s
			<< tmp.unique_stops << " unique stops, "s << tmp.actual_length << " route length, "s 
			<< tmp.real_number << " curvature"s << endl;
	}
}

void PrintStopInfo(TransportCatalogue& tc, string_view stop, ostream& out) {
	optional<set<string_view>> tmp = tc.GetStopInfo(stop);
      if (tmp == nullopt) {
		out << "Stop "sv << string(stop) << ": not found"sv << endl;
	  }else if (tmp->empty()) {
		out << "Stop "sv << string(stop) << ": no buses "sv << endl;
	  }else{
			out << "Stop "sv << string(stop) << ": buses"sv;
			for (auto& str : *tmp) {
				out << ' ' << string(str);
			}
		    out << endl;
	  }
}

void LoadInfo(TransportCatalogue& tc, istream& is, ostream& out) {
    int num;
    string s;
    is >> num;
    int i = 0;
    getline(cin, s);
    while (i < num) {
        ++i;
        getline(cin, s);
        size_t pos = s.find(' ');
        string com = s.substr(0, pos);
		string str = s.substr(pos + 1);
        if (com == "Stop"s) { 
			PrintStopInfo(tc, str, out);
		}
        if (com == "Bus"s) {
			PrintBusInfo(tc, str,out);
        }
    }
}