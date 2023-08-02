#include "input_reader.h"

using namespace std;

map<pair<string, string>, int> stop_distance;

string_view Unquote(string_view value) {
    while (!value.empty() && (value.front() == ' ' || value.back() == ' ')) {
        if (value.front() == ' ') {
            value.remove_prefix(1);
        }
        if (value.back() == ' ') {
            value.remove_suffix(1);
        }
    }
    return value;
}

void ParsingStop( TransportCatalogue& tc, string_view stop) {

    size_t pos = stop.find(' ');
    stop.remove_prefix(pos);
    pos = stop.find(':');
    string_view name = Unquote(stop.substr(0, pos));
    stop.remove_prefix(pos + 1); //рез после имени
    pos = stop.find(',');
    string lat = string(stop.substr(0, pos));
    stop.remove_prefix(pos + 1);
    pos = stop.find(',');
    string lon;
    if (pos != stop.size()) {
        lon = string(stop.substr(0, pos));
        while (pos < stop.size()) {
            stop.remove_prefix(pos + 1);
            pos = stop.find(',');
            size_t pos1 = stop.find('m');
            size_t pos2 = stop.find('o')+1;
            string_view distance = Unquote(stop.substr(0, pos1));
            string_view stop1 = Unquote( (pos == stop.size()) ? stop.substr(pos2) : stop.substr(pos2, pos - pos2));
           // cout << "Stop :" << name << ',' << distance << ',' << stop1 << '.' << endl;
            int dis = stoi(string(distance));
            string st = string(stop1);
            string na = string(name);
            stop_distance[{na, st}] = dis;
        }
        

    }
    else {
        lon = string(stop.substr(0));
    }
    tc.AddStop(name, stod( lat), stod(lon));

   // cout <<"Stop :" << string(name) << ';' << lat << ';' << lon << endl;
    //cout << stop << ',' << Unquote(name) << ';' << lon << ';' << lat << endl;
}

pair<string,vector<string>> ParsingBus(string_view bus, char by) {
    vector<string> stops;
    string name;
    size_t pos = bus.find(' ');
    bus.remove_prefix(pos);
    size_t pos1 = bus.find(':');
    name = string(Unquote(bus.substr(0, pos1)));
    bus.remove_prefix(pos1+1);
    size_t pos2 = bus.find(by);
    while (pos2 < bus.size()) {
       // cout << string(bus) << ';' << bus.substr(0, pos1) << ':' << pos1 << endl;
        stops.push_back(string(Unquote ( bus.substr(0, pos2))));
        bus.remove_prefix(pos2+1);
         pos2 = bus.find(by);
    }
    stops.push_back(string(Unquote(bus.substr(0, pos2))));
   // cout << "Bus :" << name << endl;
   // cout << string(bus) <<';'<< name  << endl;
    return { name, stops };
}


void Load(TransportCatalogue& tc) {
    map<string, vector<string>> bus;
    map<string, vector<string>> bus_ring;
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
        if (com == "Stop"s) {ParsingStop(tc, s); }
        
        if (com == "Bus"s) {
            
            if (s.find('-') < s.size()) {
                bus.insert(move( ParsingBus( s, '-')));
                 //cout <<'-' << string(s) << endl;
            }
            if (s.find('>') < s.size()) {
                bus_ring.insert(move( ParsingBus( s, '>')));
                // cout <<'>' << string(s) << endl;
            }
             //cout << string(bus) << endl;
        }
    }
    for (auto [number, stops] : bus) {
        vector<string_view> stops1;
        for (auto str : stops) {
            if (tc.FindStop(str) != nullptr) {
                // cout <<number<<';'<< str<<';' << endl;
                stops1.push_back(tc.FindStop(str)->stop_name);
            }
        }
       // cout << number<<':' << endl;
        tc.AddBus(number, stops1, false);
    }
    for (auto [number, stops] : bus_ring) {
        vector<string_view> stops2;
        for (auto str1 : stops) {
            if (tc.FindStop(str1)!=nullptr) {
                stops2.push_back(tc.FindStop(str1)->stop_name);
            }
        }
       // cout << number << endl;
      //  cout << stops2.size() << endl;
        tc.AddBus(number, stops2, true);
        
    }

    for (auto [stop, dis] : stop_distance) {
        tc.AddDistanceStop(stop.first, stop.second, dis);
    }
}
