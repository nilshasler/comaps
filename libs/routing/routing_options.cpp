#include "routing/routing_options.hpp"

#include "platform/settings.hpp"

#include "indexer/classificator.hpp"

#include "base/assert.hpp"
#include "base/checked_cast.hpp"
#include "base/string_utils.hpp"

#include <sstream>

namespace routing
{
using namespace std;

// RoutingOptions -------------------------------------------------------------------------------------

std::string_view constexpr kAvoidRoutingOptionSettingsForCar = "avoid_routing_options_car";
std::string_view constexpr kAvoidRoutingOptionSettingsForBicycle = "avoid_routing_options_bicycle";
std::string_view constexpr kAvoidRoutingOptionSettingsForPedestrian = "avoid_routing_options_pedestrian";
std::string_view constexpr kAvoidRoutingOptionSettingsForTransit = "avoid_routing_options_transit";

// static
RoutingOptions RoutingOptions::LoadOptionsFromSettings(VehicleType type)
{
  uint32_t mode = 0;
  std::string_view settingsName;
  switch (type)
  {
  case VehicleType::Car: settingsName = kAvoidRoutingOptionSettingsForCar; break;
  case VehicleType::Bicycle: settingsName = kAvoidRoutingOptionSettingsForBicycle; break;
  case VehicleType::Pedestrian: settingsName = kAvoidRoutingOptionSettingsForPedestrian; break;
  case VehicleType::Transit: settingsName = kAvoidRoutingOptionSettingsForTransit; break;
  }
  
  if (!settings::Get(settingsName, mode))
    mode = 0;

  return RoutingOptions(base::checked_cast<OptionType>(mode), type);
}

// static
void RoutingOptions::SaveOptionsToSettings(RoutingOptions options)
{
  std::string_view settingsName;
  switch (options.m_vehicle)
  {
  case VehicleType::Car: settingsName = kAvoidRoutingOptionSettingsForCar; break;
  case VehicleType::Bicycle: settingsName = kAvoidRoutingOptionSettingsForBicycle; break;
  case VehicleType::Pedestrian: settingsName = kAvoidRoutingOptionSettingsForPedestrian; break;
  case VehicleType::Transit: settingsName  = kAvoidRoutingOptionSettingsForTransit; break;
  }
  
  settings::Set(settingsName, strings::to_string(static_cast<int32_t>(options.GetOptions())));
}

void RoutingOptions::Add(RoutingOptions::Option type)
{
  if (type == RoutingOptions::AvoidPaved)
    Remove(RoutingOptions::AvoidDirty);
  else if (type == RoutingOptions::AvoidDirty)
    Remove(RoutingOptions::AvoidPaved);
  m_options |= static_cast<OptionType>(type);
}

void RoutingOptions::Remove(RoutingOptions::Option type)
{
  m_options &= ~static_cast<OptionType>(type);
}

bool RoutingOptions::Has(RoutingOptions::Option type) const
{
  return (m_options & static_cast<OptionType>(type)) != 0;
}

// RoutingOptionsClassifier ---------------------------------------------------------------------------

RoutingOptionsClassifier::RoutingOptionsClassifier()
{
  Classificator const & c = classif();

  pair<vector<string>, RoutingOptions::Option> const types[] = {
      {{"highway", "motorway"}, RoutingOptions::AvoidMotorway},

      {{"hwtag", "toll"}, RoutingOptions::AvoidToll},

      {{"route", "ferry"}, RoutingOptions::AvoidFerry},

      {{"highway", "track"}, RoutingOptions::AvoidDirty},
      {{"highway", "road"}, RoutingOptions::AvoidDirty},
      {{"psurface", "unpaved_bad"}, RoutingOptions::AvoidDirty},
      {{"psurface", "unpaved_good"}, RoutingOptions::AvoidDirty},
      {{"highway", "steps"}, RoutingOptions::AvoidSteps},
      {{"highway", "ladder"}, RoutingOptions::AvoidSteps},
      {{"psurface", "paved_good"}, RoutingOptions::AvoidPaved},
      {{"psurface", "paved_bad"}, RoutingOptions::AvoidPaved}};

  m_data.reserve(std::size(types));
  for (auto const & data : types)
    m_data.insert({c.GetTypeByPath(data.first), data.second});
}

optional<RoutingOptions::Option> RoutingOptionsClassifier::Get(uint32_t type) const
{
  ftype::TruncValue(type, 2);  // in case of highway-motorway-bridge

  auto const it = m_data.find(type);
  if (it != m_data.cend())
    return it->second;
  return {};
}

RoutingOptionsClassifier const & RoutingOptionsClassifier::Instance()
{
  static RoutingOptionsClassifier instance;
  return instance;
}

RoutingOptions::Option ChooseMainRoutingOption(RoutingOptions options, bool isCarRouter)
{
  if (isCarRouter && options.Has(RoutingOptions::AvoidToll))
    return RoutingOptions::AvoidToll;

  if (options.Has(RoutingOptions::AvoidFerry))
    return RoutingOptions::AvoidFerry;

  if (options.Has(RoutingOptions::AvoidDirty))
    return RoutingOptions::AvoidDirty;

  if (options.Has(RoutingOptions::AvoidMotorway))
    return RoutingOptions::AvoidMotorway;

  if (options.Has(RoutingOptions::AvoidSteps))
    return RoutingOptions::AvoidSteps;

  if (options.Has(RoutingOptions::AvoidPaved))
    return RoutingOptions::AvoidPaved;

  return RoutingOptions::Usual;
}

string DebugPrint(RoutingOptions const & routingOptions)
{
  ostringstream ss;
  ss << "RoutingOptions: {";

  bool wasAppended = false;
  auto const append = [&](RoutingOptions::Option road)
  {
    if (routingOptions.Has(road))
    {
      wasAppended = true;
      ss << " | " << DebugPrint(road);
    }
  };

  append(RoutingOptions::Usual);
  append(RoutingOptions::AvoidToll);
  append(RoutingOptions::AvoidMotorway);
  append(RoutingOptions::AvoidFerry);
  append(RoutingOptions::AvoidDirty);
  append(RoutingOptions::AvoidSteps);
  append(RoutingOptions::AvoidPaved);
  append(RoutingOptions::AvoidHills);
  append(RoutingOptions::AvoidRailroadXing);
  append(RoutingOptions::AvoidFord);
  append(RoutingOptions::Ebike);

  if (wasAppended)
    ss << " | ";
    
  switch (routingOptions.GetVehicleType())
  {
  case VehicleType::Car: ss << "car "; break;
  case VehicleType::Transit: ss << "transit "; break;
  case VehicleType::Bicycle:
    switch (routingOptions.GetBicycleMode())
    {
    case RoutingOptions::CyclingDefault: ss << "cycling "; break;
    case RoutingOptions::CyclingRoad: ss << "road cycling "; break;
    case RoutingOptions::CyclingGravel: ss << "gravel cycling "; break;
    case RoutingOptions::CyclingMountainBike: ss << "MTB cycling "; break;
    }
    break;
  case VehicleType::Pedestrian:
    switch (routingOptions.GetPedestrianMode())
    {
    case RoutingOptions::WalkingDefault: ss << "walking "; break;
    case RoutingOptions::WalkingHiking: ss << "hiking "; break;
    case RoutingOptions::WalkingHardHiking: ss << "hard hiking "; break;
    case RoutingOptions::WalkingStrolling: ss << "strolling "; break;
    }
    break;
  }

  ss << "}";

  return ss.str();
}

string DebugPrint(RoutingOptions::Option type)
{
  switch (type)
  {
  case RoutingOptions::Usual: return "usual";
  case RoutingOptions::AvoidToll: return "toll";
  case RoutingOptions::AvoidMotorway: return "motorway";
  case RoutingOptions::AvoidFerry: return "ferry";
  case RoutingOptions::AvoidDirty: return "dirty";
  case RoutingOptions::AvoidSteps: return "steps";
  case RoutingOptions::AvoidPaved: return "paved";
  case RoutingOptions::AvoidFord: return "ford";
  case RoutingOptions::AvoidRailroadXing: return "X-ing";
  case RoutingOptions::AvoidHills: return "hills";
  case RoutingOptions::Ebike: return "ebike";
  case RoutingOptions::Max: return "max";
  }

  UNREACHABLE();
}

RoutingOptionSetter::RoutingOptionSetter(RoutingOptions::OptionType roadsMask, VehicleType type)
{
  m_saved = RoutingOptions::LoadOptionsFromSettings(type);
  RoutingOptions::SaveOptionsToSettings(RoutingOptions(roadsMask, type));
}

RoutingOptionSetter::~RoutingOptionSetter()
{
  RoutingOptions::SaveOptionsToSettings(m_saved);
}

}  // namespace routing
