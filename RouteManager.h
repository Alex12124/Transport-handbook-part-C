#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <set>
#include <cmath>
#include <optional>

struct BusStats {
  int stop_count;
  int unique_stop_count;
  double route_distance;
  double curvature;
};

struct DistanceToStop {

  DistanceToStop(double distance_, std::string stop_name_)
    :distance(distance_), stop_name(std::move(stop_name_)){}
  double distance;
  std::string stop_name;
};

struct Coords {
  long double latitude;
  long double longitude;
};

class StopDataBase {
public:
  Coords GetCoords() const{
	return coords;
  }

  void SetCoords(const Coords& coords_) {
	coords = coords_;
  }

  std::set<std::string>& GetBuses() {
  	return buses;
  }

  const std::set<std::string>& GetBuses() const {
	return buses;
  }

  std::unordered_map<std::string, double>& GetDistance() {
  	return distance;
  }

  const std::unordered_map<std::string, double>& GetDistance() const {
	return distance;
  }
private:
  Coords coords;
  std::set<std::string> buses;
  std::unordered_map<std::string, double> distance;
};

//---------------------Pattern Strategy-----------------------//
class Strategy {
public:
  virtual ~Strategy() = default;
  virtual int ComputeStopsOnRoute(const std::vector<std::string>& stops) const = 0;

  virtual std::pair<int, double> ComputeDistancesOnRoute(const std::vector<std::string>& stops,
		  const std::unordered_map<std::string, StopDataBase>& stop_db) const  = 0;

  double ComputeDistance(const Coords& lhs, const Coords& rhs) const;

  int ComputeUniqueStopsOnRoute(const std::vector<std::string>& stops) const;

  void FillBusesInStopDB(const std::vector<std::string>& stops,
		  const std:: string& bus_name,
		  std::unordered_map<std::string, StopDataBase>& stop_db) {
	for(const std::string& stop_name: stops) {
	  stop_db[stop_name].GetBuses().insert(bus_name);
	}
  }
};

class CycleStrategy : public Strategy {
public:
  int ComputeStopsOnRoute(const std::vector<std::string>& stops) const override {
    return stops.size();
  }

  std::pair<int, double> ComputeDistancesOnRoute(const std::vector<std::string>& stops,
		  const std::unordered_map<std::string, StopDataBase>& stop_db) const override {
    double sum = 0;
    int real_sum = 0;
	for(auto it = begin(stops); it != prev(end(stops)); ++it) {
	  sum += ComputeDistance(stop_db.at(*it).GetCoords(),
			  stop_db.at(*next(it)).GetCoords());
      real_sum += stop_db.at(*it).GetDistance().at(*next(it));
    }
	return {real_sum, sum};
  }
};

class NotCycleStrategy : public Strategy {
public:
  int ComputeStopsOnRoute(const std::vector<std::string>& stops) const override {
    return stops.size() * 2 - 1;
  }

  std::pair<int, double> ComputeDistancesOnRoute(const std::vector<std::string>& stops,
		  const std::unordered_map<std::string, StopDataBase>& stop_db) const override {
	double sum = 0;
	int real_sum = 0;
	for(auto it = begin(stops); it != prev(end(stops)); ++it) {
	  sum += 2 * ComputeDistance(stop_db.at(*it).GetCoords(),
			  stop_db.at(*next(it)).GetCoords());
	  real_sum += (stop_db.at(*it).GetDistance().at(*next(it)) +
	      		  stop_db.at(*next(it)).GetDistance().at(*it));
	}
	return {real_sum, sum};
  }
};
//---------------------Pattern Strategy-----------------------//


//---------------------Business Logic of Programm----------------//
class RouteManager {
public:
  void SetStopData(const std::string& stop_name, Coords coords,
		  const std::vector<DistanceToStop>& distances) {
	stop_db[stop_name].SetCoords(coords);
	for(const DistanceToStop& dist: distances) {
	  stop_db[stop_name].GetDistance()[dist.stop_name] = dist.distance;
	  if(!stop_db[dist.stop_name].GetDistance().count(stop_name)) {
		stop_db[dist.stop_name].GetDistance()[stop_name] = dist.distance;
	  }
	}
  }

  void SetBusData(const std::string& bus_name, const std::vector<std::string>& stops) {
	BusStats stats;
	stats.stop_count = strategy->ComputeStopsOnRoute(stops);
	stats.unique_stop_count = strategy->ComputeUniqueStopsOnRoute(stops);
	auto [real_route_distance, route_distance] =
			strategy->ComputeDistancesOnRoute(stops, stop_db);
	stats.curvature = real_route_distance / route_distance;
	stats.route_distance = real_route_distance;
    bus_stats[bus_name] = stats;
    strategy->FillBusesInStopDB(stops, bus_name, stop_db);
  }

  void SetStrategy(Strategy* strategy_) {
	strategy = strategy_;
  }

  std::optional<BusStats> GetBusStats(const std::string& bus_name) const {
	if(bus_stats.count(bus_name)) {
	  return bus_stats.at(bus_name);
	}
	return std::nullopt;
  }

  std::optional<std::set<std::string>> GetStopStats(const std::string& stop_name) {
	if(stop_db.count(stop_name)) {
	  return stop_db[stop_name].GetBuses();
	}
	return std::nullopt;

  }

private:
  std::unordered_map<std::string, BusStats> bus_stats;
  std::unordered_map<std::string, StopDataBase> stop_db;
  Strategy* strategy;
};
//---------------------Business Logic of Programm----------------//

//---------------------Tests-----------------------------------//
void TestComputeDistance();
void TestBusStats();
void TestStopStats();
//---------------------Tests-----------------------------------//
