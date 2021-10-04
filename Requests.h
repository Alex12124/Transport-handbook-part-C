#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <set>
#include <string_view>
#include <sstream>
#include "RouteManager.h"

class Visitor;
class Request;
using RequestHolder = std::unique_ptr<Request>;

//------------------Request---------------------------//
class Request {
public:
  enum class Type {
	READ_STOP,
    READ_BUS,
    MODIFY_BUS,
	MODIFY_STOP,
  };

  Request(Type type);
  static RequestHolder Create(Type type);
  virtual void ParseFrom(std::string_view input) = 0;
  virtual ~Request() = default;
  virtual void Accept(const Visitor& v) const = 0;
  const Type type;
};

const std::unordered_map<std::string_view, Request::Type> MODIFY_REQUEST_TYPE = {
    {"Bus", Request::Type::MODIFY_BUS},
    {"Stop", Request::Type::MODIFY_STOP}
};

const std::unordered_map<std::string_view, Request::Type> READ_REQUEST_TYPE = {
    {"Bus", Request::Type::READ_BUS},
	{"Stop", Request::Type::READ_STOP},
};

class ReadStopRequest : public Request {
public:
  ReadStopRequest();
  void ParseFrom(std::string_view input) override;
  void Accept(const Visitor& v) const override;

  std::string stop_name;
};

class ReadBusRequest : public Request {
public:
  ReadBusRequest();
  void ParseFrom(std::string_view input) override;
  void Accept(const Visitor& v) const override;

  std::string bus_name;
};

class ModifyBusRequest : public Request {
public:
  ModifyBusRequest();
  void ParseFrom(std::string_view input) override;
  void Accept(const Visitor& v) const override;

  std::vector<std::string> stops;
  std::string bus_name;
  bool cycle = false;
  int unique_stops;
};

class ModifyStopRequest : public Request {
public:
  ModifyStopRequest();
  void ParseFrom(std::string_view input) override;
  void Accept(const Visitor& v) const override;


  double latitude, longitude;
  std::string stop_name;
  std::vector<DistanceToStop> distances;
};
//------------------Request---------------------------------------//

//------------------Parsing Functions-----------------------------//

std::pair<std::string_view, std::optional<std::string_view>>
  SplitTwoStrict(std::string_view s, std::string_view delimiter = " ");

std::pair<std::string_view, std::string_view>
  SplitTwo(std::string_view s, std::string_view delimiter = " ");

std::string_view ReadToken(std::string_view& s, std::string_view delimiter = " ");

std::vector<std::string>
  FormVector(std::string_view s, std::string_view delimiter = " ");

double ConvertToDouble(std::string_view str);

template <typename Number>
Number ReadNumberOnLine(std::istream& stream);

std::optional<Request::Type> ConvertRequestTypeFromString(std::string_view type_str,
		const std::unordered_map<std::string_view, Request::Type>& str_to_type);

RequestHolder ParseRequest(std::string_view request_str, bool is_modify);

std::vector<RequestHolder> ReadRequests(std::istream& in_stream, bool is_modify);

std::vector<double> ProcessRequests(const std::vector<RequestHolder>& requests);

void PrintRouteResponse(const std::string bus_name, std::optional<BusStats> stats,
		std::ostream& stream);

void PrintStopResponse(const std::string stop_name, std::optional<std::set<std::string>> stats,
		std::ostream& stream);

//------------------Parsing Functions-----------------------------//

//-----------------------Visitor--------------------------------//

class Visitor {
public:
  Visitor();
  void Visit(const ReadBusRequest&) const;
  void Visit(const ModifyBusRequest&) const;
  void Visit(const ModifyStopRequest&) const;
  void Visit(const ReadStopRequest&) const;
  void SetRouteManager(RouteManager* rm_);
private:
  RouteManager* rm;
  std::unique_ptr<Strategy> cycle_strategy;
  std::unique_ptr<Strategy> not_cycle_strategy;
};

//-------------------------Tests--------------------------------//
void TestReadRequest();
