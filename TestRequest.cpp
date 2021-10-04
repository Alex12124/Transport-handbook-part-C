#include "Requests.h"
#include <iomanip>
#include "test_runner.h"

using namespace std;

void TestReadRequest() {
  stringstream ss("10\n"
	"Stop Tolstopaltsevo: 55.611087, 37.20829\n"
	"Stop Marushkino: 55.595884, 37.209755\n"
	"Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > "
	  "Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
	"Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka"
  );
  ss << setprecision(6);
  const auto requests = ReadRequests(ss, true);

  ASSERT_EQUAL(static_cast<ModifyStopRequest&>(*requests[0]).latitude, 3.1415926535 * 55.611087 / 180);
  ASSERT_EQUAL(static_cast<ModifyStopRequest&>(*requests[0]).longitude, 3.1415926535 * 37.20829 / 180);
  ASSERT_EQUAL(static_cast<ModifyStopRequest&>(*requests[0]).stop_name, "Tolstopaltsevo");

  ASSERT_EQUAL(static_cast<ModifyStopRequest&>(*requests[1]).latitude, 3.1415926535 * 55.595884 / 180);
  ASSERT_EQUAL(static_cast<ModifyStopRequest&>(*requests[1]).longitude, 3.1415926535 * 37.209755 / 180);
  ASSERT_EQUAL(static_cast<ModifyStopRequest&>(*requests[1]).stop_name, "Marushkino");

  ASSERT_EQUAL(static_cast<ModifyBusRequest&>(*requests[2]).cycle, true);
  ASSERT_EQUAL(static_cast<ModifyBusRequest&>(*requests[2]).bus_name, "256");
  ASSERT_EQUAL(static_cast<ModifyBusRequest&>(*requests[2]).stops,
  	vector<string>({"Biryulyovo Zapadnoye", "Biryusinka", "Universam",
  "Biryulyovo Tovarnaya", "Biryulyovo Passazhirskaya", "Biryulyovo Zapadnoye"}));

  ASSERT_EQUAL(static_cast<ModifyBusRequest&>(*requests[3]).cycle, false);
  ASSERT_EQUAL(static_cast<ModifyBusRequest&>(*requests[3]).bus_name, "750");
  ASSERT_EQUAL(static_cast<ModifyBusRequest&>(*requests[3]).stops,
    vector<string>({"Tolstopaltsevo", "Marushkino", "Rasskazovka"}));

}
