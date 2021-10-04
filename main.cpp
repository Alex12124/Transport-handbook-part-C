#include <iostream>
#include "Requests.h"
#include "test_runner.h"
#include "RouteManager.h"

using namespace std;

void TestAll() {
  TestRunner tr;
  RUN_TEST(tr, TestReadRequest);
  RUN_TEST(tr, TestComputeDistance);
  RUN_TEST(tr, TestBusStats);
  RUN_TEST(tr, TestStopStats);
}

void ModifyProcessing(const Visitor& visitor, const vector<RequestHolder>& requests) {
  for(const RequestHolder& r: requests) {
    if(r->type == Request::Type::MODIFY_STOP) {
	  r->Accept(visitor);
	}
  }
  for(const RequestHolder& r: requests) {
	if(r->type == Request::Type::MODIFY_BUS) {
	  r->Accept(visitor);
	}
  }
}

void ReadProcessing(const Visitor& visitor, const vector<RequestHolder>& requests) {
  for(const RequestHolder& r: requests) {
	r->Accept(visitor);
  }
}

int main() {
  TestAll();

  RouteManager rm;
  Visitor visitor;
  visitor.SetRouteManager(&rm);
  cout.precision(6);
  const auto modify_requests = ReadRequests(cin, true);
  ModifyProcessing(visitor, modify_requests);
  const auto read_requests = ReadRequests(cin, false);
  ReadProcessing(visitor, read_requests);
  return 0;
}
