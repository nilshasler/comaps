#include <jni.h>
#include "app/organicmaps/sdk/Framework.hpp"
#include "app/organicmaps/sdk/core/jni_helper.hpp"
#include "routing/routing_options.hpp"

static routing::RoutingOptions::Road makeValue(jint option)
{
  auto const road = static_cast<uint8_t>(1u << static_cast<int>(option));
  CHECK_LESS(road, static_cast<uint8_t>(routing::RoutingOptions::Road::Max), ("invalid option", option));
  return static_cast<routing::RoutingOptions::Road>(road);
}

static routing::VehicleType makeVehicle(jint vehicle)
{
  auto const v = static_cast<uint8_t>(vehicle);
  CHECK_LESS(v, static_cast<uint8_t>(routing::VehicleType::Count), ("invalid vehicle type", vehicle));
  switch (v) // this is super-ugly but java and C++ define constants differently: c.f. Router.java vehicle_mask.hpp
  {
  default:
  case 0: return routing::VehicleType::Car;
  case 2: return routing::VehicleType::Bicycle;
  case 1: return routing::VehicleType::Pedestrian;
  }
}


extern "C"
{
JNIEXPORT jboolean JNICALL Java_app_organicmaps_sdk_routing_RoutingOptions_nativeHasOption(JNIEnv *, jclass,
                                                                                           jint option, jint vehicle)
{
  CHECK(g_framework, ("Framework isn't created yet!"));
  routing::RoutingOptions routingOptions = routing::RoutingOptions::LoadOptionsFromSettings(makeVehicle(vehicle));
  routing::RoutingOptions::Road road = makeValue(option);
  return static_cast<jboolean>(routingOptions.Has(road));
}

JNIEXPORT void JNICALL Java_app_organicmaps_sdk_routing_RoutingOptions_nativeAddOption(JNIEnv *, jclass, jint option, jint vehicle)
{
  CHECK(g_framework, ("Framework isn't created yet!"));
  routing::RoutingOptions routingOptions = routing::RoutingOptions::LoadOptionsFromSettings(makeVehicle(vehicle));
  routing::RoutingOptions::Road road = makeValue(option);
  routingOptions.Add(road);
  routing::RoutingOptions::SaveOptionsToSettings(routingOptions);
}

JNIEXPORT void JNICALL Java_app_organicmaps_sdk_routing_RoutingOptions_nativeRemoveOption(JNIEnv *, jclass, jint option, jint vehicle)
{
  CHECK(g_framework, ("Framework isn't created yet!"));
  routing::RoutingOptions routingOptions = routing::RoutingOptions::LoadOptionsFromSettings(makeVehicle(vehicle));
  routing::RoutingOptions::Road road = makeValue(option);
  routingOptions.Remove(road);
  routing::RoutingOptions::SaveOptionsToSettings(routingOptions);
}
}
