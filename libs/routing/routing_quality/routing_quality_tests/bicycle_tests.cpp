#include "testing/testing.hpp"

#include "routing/routing_quality/waypoints.hpp"

using namespace routing_quality;

namespace
{
// this test fails until crossing major roads gets penalized.
// As it is, the routing engine prefers crossing a major road 3 times and use a cycleway over an unclassified road with just one crossing.
// a local claims that the unclassified road should be preferred
UNIT_TEST(Bicycle_AvoidCrossingMajorRoads_Netherlands)
{
  TEST(CheckBicycleRoute({52.349225, 6.480374} /* start */, {52.321106, 6.501113} /* finish */,
                     {{{52.337725, 6.492941}}} /* reference track */),
       ());
}



} // namespace