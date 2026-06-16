#pragma once

#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include "boost/container_hash/hash.hpp"
#include "3party/skarupke/flat_hash_map.hpp"
#include "routing/routing_quality/api/api.hpp"

namespace routing
{
class RoutingOptions
{
public:
  enum Road : uint8_t
  {
    Usual = 1u << 0,
    Toll = 1u << 1,
    Motorway = 1u << 2,
    Ferry = 1u << 3,
    Dirty = 1u << 4,
    Steps = 1u << 5,
    Paved = 1u << 6,

    Max = (1u << 6) + 1
  };

  using RoadType = std::underlying_type_t<Road>;

  RoutingOptions() = default;
  explicit RoutingOptions(RoadType mask, routing::VehicleType type) : m_options(mask), m_vehicle(type) {}

  static RoutingOptions LoadOptionsFromSettings(VehicleType type);
  static void SaveOptionsToSettings(RoutingOptions options);

  void Add(Road type);
  void Remove(Road type);
  bool Has(Road type) const;

  void setVehicleType(VehicleType vt) { m_vehicle = vt; }

  RoadType GetOptions() const { return m_options; }

private:
  RoadType m_options = 0;
  VehicleType m_vehicle = VehicleType::Car;
};

class RoutingOptionsClassifier
{
public:
  RoutingOptionsClassifier();

  std::optional<RoutingOptions::Road> Get(uint32_t type) const;
  static RoutingOptionsClassifier const & Instance();

private:
  ska::flat_hash_map<uint32_t, RoutingOptions::Road, boost::hash<uint32_t>> m_data;
};

RoutingOptions::Road ChooseMainRoutingOptionRoad(RoutingOptions options, bool isCarRouter);

std::string DebugPrint(RoutingOptions const & routingOptions);
std::string DebugPrint(RoutingOptions::Road type);

/// Options guard for debugging/tests.
class RoutingOptionSetter
{
public:
  explicit RoutingOptionSetter(RoutingOptions::RoadType roadsMask, VehicleType type);
  ~RoutingOptionSetter();

private:
  RoutingOptions m_saved;
};
}  // namespace routing
