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
  if (type == RoutingOptions::Option::AvoidPaved)
    Remove(RoutingOptions::Option::AvoidDirty);
  else if (type == RoutingOptions::Option::AvoidDirty)
    Remove(RoutingOptions::Option::AvoidPaved);
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
      {{"highway", "motorway"}, RoutingOptions::Option::AvoidMotorway},

      {{"hwtag", "toll"}, RoutingOptions::Option::AvoidToll},

      {{"route", "ferry"}, RoutingOptions::Option::AvoidFerry},

      {{"highway", "track"}, RoutingOptions::Option::AvoidDirty},
      {{"highway", "road"}, RoutingOptions::Option::AvoidDirty},
      {{"psurface", "unpaved_bad"}, RoutingOptions::Option::AvoidDirty},
      {{"psurface", "unpaved_good"}, RoutingOptions::Option::AvoidDirty},
      {{"highway", "steps"}, RoutingOptions::Option::AvoidSteps},
      {{"highway", "ladder"}, RoutingOptions::Option::AvoidSteps},
      {{"psurface", "paved_good"}, RoutingOptions::Option::AvoidPaved},
      {{"psurface", "paved_bad"}, RoutingOptions::Option::AvoidPaved}};

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
  if (isCarRouter && options.Has(RoutingOptions::Option::AvoidToll))
    return RoutingOptions::Option::AvoidToll;

  if (options.Has(RoutingOptions::Option::AvoidFerry))
    return RoutingOptions::Option::AvoidFerry;

  if (options.Has(RoutingOptions::Option::AvoidDirty))
    return RoutingOptions::Option::AvoidDirty;

  if (options.Has(RoutingOptions::Option::AvoidMotorway))
    return RoutingOptions::Option::AvoidMotorway;

  if (options.Has(RoutingOptions::Option::Steps))
    return RoutingOptions::Option::Steps;

  if (options.Has(RoutingOptions::Option::AvoidPaved))
    return RoutingOptions::Option::AvoidPaved;

  return RoutingOptions::Option::Usual;
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

  append(RoutingOptions::Option::Usual);
  append(RoutingOptions::Option::AvoidToll);
  append(RoutingOptions::Option::AvoidMotorway);
  append(RoutingOptions::Option::AvoidFerry);
  append(RoutingOptions::Option::AvoidDirty);
  append(RoutingOptions::Option::Steps);
  append(RoutingOptions::Option::AvoidPaved);

  if (wasAppended)
    ss << " | ";

  ss << "}";

  return ss.str();
}

string DebugPrint(RoutingOptions::Option type)
{
  switch (type)
  {
  case RoutingOptions::Option::AvoidToll: return "toll";
  case RoutingOptions::Option::AvoidMotorway: return "motorway";
  case RoutingOptions::Option::AvoidFerry: return "ferry";
  case RoutingOptions::Option::AvoidDirty: return "dirty";
  case RoutingOptions::Option::Steps: return "steps";
  case RoutingOptions::Option::AvoidPaved: return "paved";
  case RoutingOptions::Option::Usual: return "usual";
  case RoutingOptions::Option::Max: return "max";
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
