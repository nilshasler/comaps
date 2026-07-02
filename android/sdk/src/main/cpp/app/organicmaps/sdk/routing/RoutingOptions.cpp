#include <jni.h>
#include "app/organicmaps/sdk/Framework.hpp"
#include "app/organicmaps/sdk/core/jni_helper.hpp"
#include "routing/routing_options.hpp"

static routing::RoutingOptions::Option makeValue(jint option)
{
  auto const opt = static_cast<uint16_t>(1u << static_cast<int>(option));
  CHECK_LESS(opt, static_cast<uint16_t>(routing::RoutingOptions::Max), ("invalid option", option));
  return static_cast<routing::RoutingOptions::Option>(opt);
}

static routing::RoutingOptions::Option makeTransportSubMode(jint type)
{
  auto const t = static_cast<uint16_t>(type);
  CHECK_EQUAL(t & ~routing::RoutingOptions::SubModeMask, 0, ("invalid submode", type));
  return t;
}

static routing::VehicleType makeVehicle(jint vehicle)
{
  auto const v = static_cast<uint16_t>(vehicle);
  CHECK_LESS(v, static_cast<uint16_t>(routing::VehicleType::Count), ("invalid vehicle type", vehicle));
  switch (v)  // this is super-ugly but java and C++ define constants differently: c.f. Router.java vehicle_mask.hpp
  {
  default:
  case 0: return routing::VehicleType::Car;
  case 1: return routing::VehicleType::Pedestrian;
  case 2: return routing::VehicleType::Bicycle;
  case 3: return routing::VehicleType::Pedestrian; // transit
  }
}

extern "C"
{
JNIEXPORT jboolean JNICALL Java_app_organicmaps_sdk_routing_RoutingOptions_nativeHasOption(JNIEnv *, jclass,
                                                                                           jint option, jint vehicle)
{
  CHECK(g_framework, ("Framework isn't created yet!"));
  routing::RoutingOptions routingOptions = routing::RoutingOptions::LoadOptionsFromSettings(makeVehicle(vehicle));
  routing::RoutingOptions::Option opt = makeValue(option);
  return static_cast<jboolean>(routingOptions.Has(opt));
}

JNIEXPORT void JNICALL Java_app_organicmaps_sdk_routing_RoutingOptions_nativeAddOption(JNIEnv *, jclass, jint option,
                                                                                       jint vehicle)
{
  CHECK(g_framework, ("Framework isn't created yet!"));
  routing::RoutingOptions routingOptions = routing::RoutingOptions::LoadOptionsFromSettings(makeVehicle(vehicle));
  routing::RoutingOptions::Option opt = makeValue(option);
  routingOptions.Add(opt);
  routing::RoutingOptions::SaveOptionsToSettings(routingOptions);
}

JNIEXPORT void JNICALL Java_app_organicmaps_sdk_routing_RoutingOptions_nativeRemoveOption(JNIEnv *, jclass, jint option, jint vehicle)
{
  CHECK(g_framework, ("Framework isn't created yet!"));
  routing::RoutingOptions routingOptions = routing::RoutingOptions::LoadOptionsFromSettings(makeVehicle(vehicle));
  routing::RoutingOptions::Option opt = makeValue(option);
  routingOptions.Remove(opt);
  routing::RoutingOptions::SaveOptionsToSettings(routingOptions);
}

JNIEXPORT jint JNICALL Java_app_organicmaps_sdk_routing_RoutingOptions_nativeGetTransportSubMode(JNIEnv *, jclass, jint vehicle)
{
  CHECK(g_framework, ("Framework isn't created yet!"));
  routing::VehicleType vt = makeVehicle(vehicle);
  routing::RoutingOptions routingOptions = routing::RoutingOptions::LoadOptionsFromSettings(vt);
  return static_cast<jint>(routingOptions.GetTransportSubMode(vt));
}

JNIEXPORT void JNICALL Java_app_organicmaps_sdk_routing_RoutingOptions_nativeSetTransportSubMode(JNIEnv *, jclass, jint mode, jint vehicle)
{
  CHECK(g_framework, ("Framework isn't created yet!"));
  routing::VehicleType vt = makeVehicle(vehicle);
  routing::RoutingOptions routingOptions = routing::RoutingOptions::LoadOptionsFromSettings(vt);
  routingOptions.SetTransportSubMode(makeTransportSubMode(mode), vt);
  routing::RoutingOptions::SaveOptionsToSettings(routingOptions);
}
} // extern "C"