#pragma once

#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include "3party/skarupke/flat_hash_map.hpp"
#include "boost/container_hash/hash.hpp"
#include "routing/routing_quality/api/api.hpp"

namespace routing
{
class RoutingOptions
{
public:
  enum Option : uint16_t
  {
    Usual = 1u << 0,
    AvoidToll = 1u << 1,
    AvoidMotorway = 1u << 2,
    AvoidFerry = 1u << 3,
    AvoidDirty = 1u << 4,
    AvoidSteps = 1u << 5,
    AvoidPaved = 1u << 6,
    AvoidFord = 1u << 7,
    AvoidRailroadXing = 1u << 8,
    AvoidHills = 1u << 9,
    Ebike = 1u << 10,

    SubModeMask = 3u << 11,
    CyclingDefault = 0u << 11,
    CyclingRoad = 1u << 11,
    CyclingGravel = 2u << 11,
    CyclingMountainBike = 3u << 11,
    
    WalkingDefault = 0u << 11,
    WalkingHiking = 1u << 11,
    WalkingHardHiking = 2u << 11,
    WalkingStrolling = 3u << 11,
    

    Max = (1u << 12) + 1
  };

  using OptionType = std::underlying_type_t<Option>;

  RoutingOptions() = default;
  explicit RoutingOptions(OptionType mask, routing::VehicleType type) : m_options(mask), m_vehicle(type) {}

  static RoutingOptions LoadOptionsFromSettings(VehicleType type);
  static void SaveOptionsToSettings(RoutingOptions options);

  void Add(Option type);
  void Remove(Option type);
  bool Has(Option type) const;

  void SetCyclingMode(OptionType mode);
  OptionType GetCyclingMode() const { return (m_options & SubModeMask); }
  void SetWalkingMode(OptionType mode);
  OptionType GetWalkingMode() const { return (m_options & SubModeMask); }

  void setVehicleType(VehicleType vt) { m_vehicle = vt; }

  OptionType GetOptions() const { return m_options; }

private:
  OptionType m_options = 0;
  VehicleType m_vehicle = VehicleType::Car;
};

class RoutingOptionsClassifier
{
public:
  RoutingOptionsClassifier();

  std::optional<RoutingOptions::Option> Get(uint32_t type) const;
  static RoutingOptionsClassifier const & Instance();

private:
  ska::flat_hash_map<uint32_t, RoutingOptions::Option, boost::hash<uint32_t>> m_data;
};

RoutingOptions::Option ChooseMainRoutingOption(RoutingOptions options, bool isCarRouter);

std::string DebugPrint(RoutingOptions const & routingOptions);
std::string DebugPrint(RoutingOptions::Option type);

/// Options guard for debugging/tests.
class RoutingOptionSetter
{
public:
  explicit RoutingOptionSetter(RoutingOptions::OptionType optionssMask, VehicleType type);
  ~RoutingOptionSetter();

private:
  RoutingOptions m_saved;
};
}  // namespace routing
