#include "Requests.h"
#include <set>

using namespace std;

//------------------Request---------------------------//

Request::Request(Type type) : type(type) {}

RequestHolder Request::Create(Request::Type type) {
  switch (type) {
    case Request::Type::MODIFY_BUS:
      return std::make_unique<ModifyBusRequest>();
    case Request::Type::MODIFY_STOP:
      return std::make_unique<ModifyStopRequest>();
    case Request::Type::READ_BUS:
	  return std::make_unique<ReadBusRequest>();
    case Request::Type::READ_STOP:
    	  return std::make_unique<ReadStopRequest>();
    default:
      return nullptr;
  }
}

ReadStopRequest::ReadStopRequest() : Request(Type::READ_STOP){}

void ReadStopRequest::ParseFrom(std::string_view input) {
  stop_name = input;
}

ReadBusRequest::ReadBusRequest() : Request(Type::READ_BUS) {}

void ReadBusRequest::ParseFrom(std::string_view input) {
  bus_name = input;
}

ModifyBusRequest::ModifyBusRequest() : Request(Type::MODIFY_BUS) {}

void ModifyBusRequest::ParseFrom(std::string_view input) {
  bus_name = ReadToken(input, ": ");
  const size_t pos = input.find(" - ");
  if(pos == input.npos) {
    cycle = true;
    stops = FormVector(input, " > ");

  } else {
    stops = FormVector(input, " - ");
  }
}

ModifyStopRequest::ModifyStopRequest() : Request(Type::MODIFY_STOP) {}

void ModifyStopRequest::ParseFrom(std::string_view input) {
  stop_name = ReadToken(input, ": ");
  latitude = ConvertToDouble(ReadToken(input, ", ")) * 3.1415926535 / 180;
  longitude = ConvertToDouble(ReadToken(input, ", ")) * 3.1415926535 / 180;
  while(!input.empty()) {
	distances.push_back({ConvertToDouble(ReadToken(input, "m to ")),
		std::move(string(ReadToken(input, ", ")))});
  }

}

void ReadStopRequest::Accept(const Visitor& v) const {
  v.Visit(*this);
}

void ReadBusRequest::Accept(const Visitor& v) const {
  v.Visit(*this);
}

void ModifyBusRequest::Accept(const Visitor& v) const {
  v.Visit(*this);
}

void ModifyStopRequest::Accept(const Visitor& v) const {
  v.Visit(*this);
}

//------------------Request---------------------------//

//------------------Parsing Functions-----------------//
pair<string_view, optional<string_view>> SplitTwoStrict(string_view s, string_view delimiter) {
  const size_t pos = s.find(delimiter);
  if (pos == s.npos) {
    return {s, nullopt};
  } else {
    return {s.substr(0, pos), s.substr(pos + delimiter.length())};
  }
}

pair<string_view, string_view> SplitTwo(string_view s, string_view delimiter ) {
  const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
  return {lhs, rhs_opt.value_or("")};
}

string_view ReadToken(string_view& s, string_view delimiter) {
  const auto [lhs, rhs] = SplitTwo(s, delimiter);
  s = rhs;
  return lhs;
}

vector<string> FormVector(string_view s, string_view delimiter) {
	vector<string> stops;
	while(!s.empty()) {
		string stop_name = string(ReadToken(s, delimiter));
		stops.push_back(stop_name);
	}
	return stops;
}

double ConvertToDouble(string_view str) {
  // use std::from_chars when available to git rid of string copy
  size_t pos;
  const double result = stod(string(str), &pos);
  if (pos != str.length()) {
    std::stringstream error;
    error << "string " << str << " contains " << (str.length() - pos) << " trailing chars";
    throw invalid_argument(error.str());
  }
  return result;
}

template <typename Number>
Number ReadNumberOnLine(istream& stream) {
  Number number;
  stream >> number;
  string dummy;
  getline(stream, dummy);
  return number;
}

optional<Request::Type> ConvertRequestTypeFromString(string_view type_str,
		const unordered_map<string_view, Request::Type>& str_to_type) {

  if (const auto it = str_to_type.find(type_str);
	it != str_to_type.end()) {
	return it->second;
  } else {
	return nullopt;
  }

}

RequestHolder ParseRequest(string_view request_str, bool is_modify) {

  const auto request_type = [&request_str, is_modify] {
	string_view s = ReadToken(request_str);
	if(is_modify) {
	    return ConvertRequestTypeFromString(s, MODIFY_REQUEST_TYPE);
	}
	return ConvertRequestTypeFromString(s, READ_REQUEST_TYPE);
  }();

  if (!request_type) {
    return nullptr;
  }
  RequestHolder request = Request::Create(*request_type);
  if (request) {
    request->ParseFrom(request_str);
  };
  return request;
}

vector<RequestHolder> ReadRequests(istream& in_stream, bool is_modify) {
  const size_t request_count = ReadNumberOnLine<size_t>(in_stream);

  vector<RequestHolder> requests;
  requests.reserve(request_count);

  for (size_t i = 0; i < request_count; ++i) {
    string request_str;
    getline(in_stream, request_str);
    if (auto request = ParseRequest(request_str, is_modify)) {
      requests.push_back(move(request));
    }
  }
  return requests;
}

//------------------Parsing Functions-----------------------------//

//-----------------------PrintResults-------------------------------//

void PrintRouteResponse(const std::string bus_name, optional<BusStats> stats,
		ostream& stream) {
  if(!stats) {
	stream << "Bus " << bus_name << ": not found\n";
  }	else {
  stream << "Bus " << bus_name  << ": "<< stats->stop_count
		  << " stops on route, " << stats->unique_stop_count
		  << " unique stops, " << stats->route_distance
		  << " route length, " << stats->curvature
		  << " curvature\n";

  }
}

void PrintStopResponse(const std::string stop_name, std::optional<std::set<std::string>> stats,
		std::ostream& stream) {
  if(!stats) {
    stream << "Stop " << stop_name << ": not found\n";
  } else {
    if(stats->empty()) {
	  stream << "Stop " << stop_name << ": no buses\n";
    } else {
	  stream << "Stop " << stop_name  << ": buses";
	  for(const std::string& bus: (*stats)) {
	    stream << " " << bus;
	  }
	  stream << '\n';
  }

}
}
//-----------------------PrintResults-------------------------------//


//---------------Visitor------------------------------//

void Visitor::Visit(const ReadBusRequest& request) const {
  PrintRouteResponse(request.bus_name, rm->GetBusStats(request.bus_name), cout);
}

void Visitor::Visit(const ReadStopRequest& request) const {
  PrintStopResponse(request.stop_name, rm->GetStopStats(request.stop_name), cout);
}

void Visitor::Visit(const ModifyBusRequest& request) const {
  if(request.cycle) {
	rm->SetStrategy(cycle_strategy.get());
  } else {
	rm->SetStrategy(not_cycle_strategy.get());
  }
  rm->SetBusData(request.bus_name, request.stops);
}
void Visitor::Visit(const ModifyStopRequest& request) const {
  rm->SetStopData(request.stop_name, Coords{request.latitude, request.longitude},
		  request.distances);
}

void Visitor::SetRouteManager(RouteManager* rm_) {
  rm = rm_;
}

Visitor::Visitor() {
  cycle_strategy = make_unique<CycleStrategy>();
  not_cycle_strategy = make_unique<NotCycleStrategy>();
}

//---------------Visitor------------------------------//
