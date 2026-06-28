#include "routing_common/bicycle_model.hpp"

#include "indexer/classificator.hpp"
#include "indexer/feature_data.hpp"

#include "base/assert.hpp"

namespace bicycle_model
{
using namespace routing;

// See model specifics in different countries here:
//   https://wiki.openstreetmap.org/wiki/OSM_tags_for_routing/Access-Restrictions
// Document contains proposals for some countries, but we assume that some kinds of roads are ready for bicycle routing,
// but not listed in tables in the document. For example, steps are not listed, paths, roads and services features also
// can be treated as ready for bicycle routing. These road types were added to lists below.

// See road types here:
//   https://wiki.openstreetmap.org/wiki/Key:highway

// Heuristics:
// For less bicycle roads we add fine by setting smaller value of weight speed, and for more bicycle roads we
// set greater values of weight speed. Algorithm picks roads with greater weight speed first,
// preferencing a more bicycle roads over less bicycle.
// As result of such heuristic road is not totally the shortest, but it avoids non bicycle roads, which were
// not marked as "hwtag=nobicycle" in OSM.

HighwayBasedFactors const kDefaultFactors = GetOneFactorsForBicycleAndPedestrianModel();

HighwayBasedFactors const& GetDefaultFactors(RoutingOptions::OptionType mode)
{
  switch (mode)
  {
  default:
  case RoutingOptions::CyclingDefault:
  case RoutingOptions::CyclingRoad:
    return kDefaultFactors;
  case RoutingOptions::CyclingGravel:
  case RoutingOptions::CyclingMountainBike:
    return kDefaultFactors;
  }
}

SpeedKMpH constexpr kSpeedOffroadKMpH = {1.5 /* weight */, 3.0 /* eta */};
SpeedKMpH constexpr kSpeedDismountKMpH = {2.0 /* weight */, 4.0 /* eta */};
// Applies only to countries where cycling is allowed on footways (by default the above dismount speed is used).
SpeedKMpH constexpr kSpeedOnFootwayKMpH = {8.0 /* weight */, 10.0 /* eta */};

HighwayBasedSpeeds const kDefaultSpeeds = {
    // {highway class : InOutCitySpeedKMpH(in city(weight, eta), out city(weight eta))}
    // Note that roads with hwtag=yesbicycle get high speed of 0.9 * Cycleway.
    /// @see Russia_UseTrunk test for Trunk weights.
    {HighwayType::HighwayTrunk, InOutCitySpeedKMpH(SpeedKMpH(7.0, 17.0), SpeedKMpH(9.0, 19.0))},
    // Presence of link roads usually means that connected roads are high traffic.
    // And complex intersections themselves are not nice for cyclists. We can't
    // easily extrapolate this to the main roads, but at least penalize the link roads a bit.
    // https://github.com/organicmaps/organicmaps/pull/9692#discussion_r1851442568
    {HighwayType::HighwayTrunkLink, InOutCitySpeedKMpH(SpeedKMpH(6.0, 17.0), SpeedKMpH(8.0, 19.0))},
    {HighwayType::HighwayPrimary, InOutCitySpeedKMpH(SpeedKMpH(10.0, 17.0), SpeedKMpH(12.0, 19.0))},
    {HighwayType::HighwayPrimaryLink, InOutCitySpeedKMpH(SpeedKMpH(8.0, 17.0), SpeedKMpH(11.0, 19.0))},
    {HighwayType::HighwaySecondary, InOutCitySpeedKMpH(SpeedKMpH(13.0, 17.0), SpeedKMpH(15.0, 19.0))},
    {HighwayType::HighwaySecondaryLink, InOutCitySpeedKMpH(SpeedKMpH(11.0, 17.0), SpeedKMpH(13.0, 19.0))},
    {HighwayType::HighwayTertiary, InOutCitySpeedKMpH(SpeedKMpH(14.0, 17.0), SpeedKMpH(17.0, 19.0))},
    {HighwayType::HighwayTertiaryLink, InOutCitySpeedKMpH(SpeedKMpH(13.0, 17.0), SpeedKMpH(16.0, 19.0))},
    {HighwayType::HighwayUnclassified, InOutCitySpeedKMpH(SpeedKMpH(13.0, 17.0), SpeedKMpH(15.0, 19.0))},
    {HighwayType::HighwayResidential, InOutCitySpeedKMpH(SpeedKMpH(12.0, 14.0), SpeedKMpH(14.0, 17.0))},
    {HighwayType::HighwayService, InOutCitySpeedKMpH(SpeedKMpH(13.0, 15.0), SpeedKMpH(15.0, 17.0))},
    {HighwayType::HighwayRoad, InOutCitySpeedKMpH(SpeedKMpH(11.0, 15.0), SpeedKMpH(14.0, 17.0))},

    {HighwayType::HighwayTrack, InOutCitySpeedKMpH(SpeedKMpH(8.0, 12.0), SpeedKMpH(10.0, 14.0))},
    {HighwayType::HighwayPath, InOutCitySpeedKMpH(SpeedKMpH(6.0, 10.0), SpeedKMpH(7.0, 12.0))},
    {HighwayType::HighwayBridleway, InOutCitySpeedKMpH(SpeedKMpH(4.0, 10.0), SpeedKMpH(5.0, 12.0))},

    {HighwayType::HighwayCycleway, InOutCitySpeedKMpH(SpeedKMpH(21.0, 18.0), SpeedKMpH(23.0, 20.0))},
    {HighwayType::HighwayLivingStreet, InOutCitySpeedKMpH(SpeedKMpH(12.0, 10.0), SpeedKMpH(14.0, 12.0))},
    // Steps have obvious inconvenience of a bike in hands.
    {HighwayType::HighwaySteps, InOutCitySpeedKMpH(SpeedKMpH(1.0, 1.0))},
    {HighwayType::HighwayPedestrian, InOutCitySpeedKMpH(kSpeedDismountKMpH)},
    {HighwayType::HighwayFootway, InOutCitySpeedKMpH(kSpeedDismountKMpH)},
    {HighwayType::ManMadePier, InOutCitySpeedKMpH(kSpeedOnFootwayKMpH)},
    /// @todo A car ferry has {10, 10}. Weight = 9 is 60% from reasonable 15 average speed.
    {HighwayType::RouteFerry, InOutCitySpeedKMpH(SpeedKMpH(9.0, 20.0))},
};

HighwayBasedSpeeds const kGravelSpeeds = {
    // {highway class : InOutCitySpeedKMpH(in city(weight, eta), out city(weight eta))}
    // Note that roads with hwtag=yesbicycle get high speed of 0.9 * Cycleway.
    /// @see Russia_UseTrunk test for Trunk weights.
    {HighwayType::HighwayTrunk, InOutCitySpeedKMpH(SpeedKMpH(2.0, 17.0), SpeedKMpH(4.0, 19.0))},
    // Presence of link roads usually means that connected roads are high traffic.
    // And complex intersections themselves are not nice for cyclists. We can't
    // easily extrapolate this to the main roads, but at least penalize the link roads a bit.
    // https://github.com/organicmaps/organicmaps/pull/9692#discussion_r1851442568
    {HighwayType::HighwayTrunkLink, InOutCitySpeedKMpH(SpeedKMpH(2.0, 17.0), SpeedKMpH(4.0, 19.0))},
    {HighwayType::HighwayPrimary, InOutCitySpeedKMpH(SpeedKMpH(3.0, 17.0), SpeedKMpH(5.0, 19.0))},
    {HighwayType::HighwayPrimaryLink, InOutCitySpeedKMpH(SpeedKMpH(4.0, 17.0), SpeedKMpH(6.0, 19.0))},
    {HighwayType::HighwaySecondary, InOutCitySpeedKMpH(SpeedKMpH(4.0, 17.0), SpeedKMpH(6.0, 19.0))},
    {HighwayType::HighwaySecondaryLink, InOutCitySpeedKMpH(SpeedKMpH(4.0, 17.0), SpeedKMpH(6.0, 19.0))},
    {HighwayType::HighwayTertiary, InOutCitySpeedKMpH(SpeedKMpH(7.0, 17.0), SpeedKMpH(9.0, 19.0))},
    {HighwayType::HighwayTertiaryLink, InOutCitySpeedKMpH(SpeedKMpH(6.0, 17.0), SpeedKMpH(8.0, 19.0))},
    {HighwayType::HighwayUnclassified, InOutCitySpeedKMpH(SpeedKMpH(7.0, 17.0), SpeedKMpH(15.0, 19.0))},
    {HighwayType::HighwayResidential, InOutCitySpeedKMpH(SpeedKMpH(7.0, 14.0), SpeedKMpH(14.0, 17.0))},
    {HighwayType::HighwayService, InOutCitySpeedKMpH(SpeedKMpH(7.0, 15.0), SpeedKMpH(15.0, 17.0))},
    {HighwayType::HighwayRoad, InOutCitySpeedKMpH(SpeedKMpH(7.0, 15.0), SpeedKMpH(14.0, 17.0))},

    {HighwayType::HighwayTrack, InOutCitySpeedKMpH(SpeedKMpH(12.0, 12.0), SpeedKMpH(15.0, 14.0))},
    {HighwayType::HighwayPath, InOutCitySpeedKMpH(SpeedKMpH(10.0, 10.0), SpeedKMpH(12.0, 12.0))},
    {HighwayType::HighwayBridleway, InOutCitySpeedKMpH(SpeedKMpH(4.0, 10.0), SpeedKMpH(5.0, 12.0))},

    {HighwayType::HighwayCycleway, InOutCitySpeedKMpH(SpeedKMpH(10.0, 18.0), SpeedKMpH(14.0, 20.0))},
    {HighwayType::HighwayLivingStreet, InOutCitySpeedKMpH(SpeedKMpH(7.0, 10.0), SpeedKMpH(10.0, 12.0))},
    // Steps have obvious inconvenience of a bike in hands.
    {HighwayType::HighwaySteps, InOutCitySpeedKMpH(SpeedKMpH(1.0, 1.0))},
    {HighwayType::HighwayPedestrian, InOutCitySpeedKMpH(kSpeedDismountKMpH)},
    {HighwayType::HighwayFootway, InOutCitySpeedKMpH(kSpeedDismountKMpH)},
    {HighwayType::ManMadePier, InOutCitySpeedKMpH(kSpeedOnFootwayKMpH)},
    /// @todo A car ferry has {10, 10}. Weight = 9 is 60% from reasonable 15 average speed.
    {HighwayType::RouteFerry, InOutCitySpeedKMpH(SpeedKMpH(6.0, 20.0))},
};

HighwayBasedSpeeds const& GetDefaultSpeeds(RoutingOptions::OptionType mode)
{
  switch (mode)
  {
  default:
  case RoutingOptions::CyclingDefault:
  //case RoutingOptions::CyclingRoad:
    return kDefaultSpeeds;
  case RoutingOptions::CyclingGravel:
  //case RoutingOptions::CyclingMountainBike:
    return kGravelSpeeds;
  }
}

// Default, no bridleway.
VehicleModel::LimitsInitList const kDefaultOptions = {
    // {HighwayType, passThroughAllowed}
    {HighwayType::HighwayTrunk, true},
    {HighwayType::HighwayTrunkLink, true},
    {HighwayType::HighwayPrimary, true},
    {HighwayType::HighwayPrimaryLink, true},
    {HighwayType::HighwaySecondary, true},
    {HighwayType::HighwaySecondaryLink, true},
    {HighwayType::HighwayTertiary, true},
    {HighwayType::HighwayTertiaryLink, true},
    {HighwayType::HighwayService, true},
    {HighwayType::HighwayUnclassified, true},
    {HighwayType::HighwayRoad, true},
    {HighwayType::HighwayTrack, true},
    {HighwayType::HighwayPath, true},
    // HighwayBridleway is missing
    {HighwayType::HighwayCycleway, true},
    {HighwayType::HighwayResidential, true},
    {HighwayType::HighwayLivingStreet, true},
    // HighwayLadder is missing
    {HighwayType::HighwaySteps, true},
    {HighwayType::HighwayPedestrian, true},
    {HighwayType::HighwayFootway, true},
    {HighwayType::ManMadePier, true},
    {HighwayType::RouteFerry, true}};

// Same as defaults except trunk and trunk_link are not allowed
VehicleModel::LimitsInitList NoTrunk()
{
  VehicleModel::LimitsInitList res;
  res.reserve(kDefaultOptions.size() - 2);
  for (auto const & e : kDefaultOptions)
    if (e.m_type != HighwayType::HighwayTrunk && e.m_type != HighwayType::HighwayTrunkLink)
      res.push_back(e);
  return res;
}

// Same as defaults except pedestrian is allowed
HighwayBasedSpeeds NormalPedestrianSpeed(RoutingOptions::OptionType mode)
{
  HighwayBasedSpeeds res = bicycle_model::GetDefaultSpeeds(mode);
  res[HighwayType::HighwayPedestrian] = InOutCitySpeedKMpH(kSpeedOnFootwayKMpH);
  return res;
}

// Same as defaults except bridleway is allowed
VehicleModel::LimitsInitList AllAllowed()
{
  auto res = kDefaultOptions;
  res.push_back({HighwayType::HighwayBridleway, true});
  return res;
}

// Same as defaults except pedestrian and footway are allowed
HighwayBasedSpeeds NormalPedestrianAndFootwaySpeed(RoutingOptions::OptionType mode)
{
  HighwayBasedSpeeds res = bicycle_model::GetDefaultSpeeds(mode);
  InOutCitySpeedKMpH const footSpeed(kSpeedOnFootwayKMpH);
  res[HighwayType::HighwayPedestrian] = footSpeed;
  res[HighwayType::HighwayFootway] = footSpeed;
  return res;
}

HighwayBasedSpeeds DismountPathSpeed(RoutingOptions::OptionType mode)
{
  HighwayBasedSpeeds res = bicycle_model::GetDefaultSpeeds(mode);
  res[HighwayType::HighwayPath] = InOutCitySpeedKMpH(kSpeedDismountKMpH);
  return res;
}

HighwayBasedSpeeds PreferFootwaysToRoads(RoutingOptions::OptionType mode)
{
  HighwayBasedSpeeds res = bicycle_model::GetDefaultSpeeds(mode);

  // Decrease secondary/tertiary weight speed (-20% from default).
  InOutCitySpeedKMpH roadSpeed = InOutCitySpeedKMpH(SpeedKMpH(11.0, 17.0), SpeedKMpH(16.0, 19.0));
  res[HighwayType::HighwaySecondary] = roadSpeed;
  res[HighwayType::HighwaySecondaryLink] = roadSpeed;
  res[HighwayType::HighwayTertiary] = roadSpeed;
  res[HighwayType::HighwayTertiaryLink] = roadSpeed;

  // Increase footway speed to make bigger than other roads (+20% from default roads).
  InOutCitySpeedKMpH footSpeed = InOutCitySpeedKMpH(SpeedKMpH(17.0, 12.0), SpeedKMpH(20.0, 15.0));
  res[HighwayType::HighwayPedestrian] = footSpeed;
  res[HighwayType::HighwayFootway] = footSpeed;

  return res;
}

// No trunk, No pass through living_street and service
VehicleModel::LimitsInitList UkraineOptions()
{
  auto res = NoTrunk();
  for (auto & e : res)
    if (e.m_type == HighwayType::HighwayLivingStreet || e.m_type == HighwayType::HighwayService)
      e.m_isPassThroughAllowed = false;
  return res;
}

VehicleModel::SurfaceInitList const kDefaultSurface = {
    // {{surfaceType}, {weightFactor, etaFactor}}
    {{"psurface", "paved_good"}, {1.0, 1.0}},
    {{"psurface", "paved_bad"}, {0.8, 0.8}},
    {{"psurface", "unpaved_good"}, {0.9, 0.9}},
    {{"psurface", "unpaved_bad"}, {0.5, 0.3}},
    // No dedicated cycleway doesn't mean that bicycle is not allowed, just lower weight.
    // If nocycleway is tagged explicitly then there is no cycling infra for sure.
    // Otherwise there is a small chance cycling infra is present though not mapped?
    /// @todo(pastk): this heuristic is controversial, maybe remove completely?
    {{"hwtag", "nocycleway"}, {0.95, 0.95}},
};
VehicleModel::SurfaceInitList const kGravelSurface = {
    // {{surfaceType}, {weightFactor, etaFactor}}
    {{"psurface", "paved_good"}, {0.7, 1.0}},
    {{"psurface", "paved_bad"}, {0.7, 0.8}},
    {{"psurface", "unpaved_good"}, {1.0, 0.9}},
    {{"psurface", "unpaved_bad"}, {0.3, 0.3}},
    // No dedicated cycleway doesn't mean that bicycle is not allowed, just lower weight.
    // If nocycleway is tagged explicitly then there is no cycling infra for sure.
    // Otherwise there is a small chance cycling infra is present though not mapped?
    /// @todo(pastk): this heuristic is controversial, maybe remove completely?
    {{"hwtag", "nocycleway"}, {0.95, 0.95}},
};

VehicleModel::SurfaceInitList const& GetDefaultSurface(RoutingOptions::OptionType mode)
{
  switch (mode)
  {
  default:
  case RoutingOptions::CyclingDefault:
  //case RoutingOptions::CyclingRoad:
    return kDefaultSurface;
  case RoutingOptions::CyclingGravel:
  //case RoutingOptions::CyclingMountainBike:
    return kGravelSurface;
  }
}


}  // namespace bicycle_model

namespace routing
{
BicycleModel::BicycleModel(RoutingOptions::OptionType mode) : BicycleModel(mode, bicycle_model::kDefaultOptions) {}

BicycleModel::BicycleModel(RoutingOptions::OptionType mode, VehicleModel::LimitsInitList const & limits)
  : BicycleModel(mode, limits, bicycle_model::GetDefaultSpeeds(mode))
{}

BicycleModel::BicycleModel(RoutingOptions::OptionType mode, VehicleModel::LimitsInitList const & limits, HighwayBasedSpeeds const & speeds)
  : VehicleModel(classif(), limits, bicycle_model::GetDefaultSurface(mode), {speeds, bicycle_model::GetDefaultFactors(mode)})
{
  using namespace bicycle_model;
  
  // No bridleway in default.
  ASSERT_EQUAL(kDefaultOptions.size(), kDefaultSpeeds.size() - 1, ());

  std::vector<std::string> hwtagYesBicycle = {"hwtag", "yesbicycle"};

  auto const & cl = classif();
  m_noType = cl.GetTypeByPath({"hwtag", "nobicycle"});
  m_yesType = cl.GetTypeByPath(hwtagYesBicycle);
  m_bidirBicycleType = cl.GetTypeByPath({"hwtag", "bidir_bicycle"});
  m_onedirBicycleType = cl.GetTypeByPath({"hwtag", "onedir_bicycle"});

  // Assign 90% of max cycleway speed for bicycle=yes to keep choosing most preferred cycleway.
  auto const yesSpeed = kDefaultSpeeds.at(HighwayType::HighwayCycleway).m_inCity * 0.9;
  AddAdditionalRoadTypes(cl, {{std::move(hwtagYesBicycle), InOutCitySpeedKMpH(yesSpeed)}});

  // Update max speed with possible ferry transfer and bicycle speed downhill.
  // See EdgeEstimator::CalcHeuristic, GetBicycleClimbPenalty.
  SpeedKMpH constexpr kMaxBicycleSpeedKMpH(100.0);
  CHECK_LESS(m_maxModelSpeed, kMaxBicycleSpeedKMpH, ());
  m_maxModelSpeed = kMaxBicycleSpeedKMpH;
}

bool BicycleModel::IsBicycleBidir(feature::TypesHolder const & types) const
{
  return types.Has(m_bidirBicycleType);
}

bool BicycleModel::IsBicycleOnedir(feature::TypesHolder const & types) const
{
  return types.Has(m_onedirBicycleType);
}

SpeedKMpH BicycleModel::GetSpeed(FeatureTypes const & types, SpeedParams const & speedParams) const
{
  return GetTypeSpeedImpl(types, speedParams, false /* isCar */);
}

bool BicycleModel::IsOneWay(FeatureTypes const & types) const
{
  if (IsBicycleOnedir(types))
    return true;

  if (IsBicycleBidir(types))
    return false;

  return VehicleModel::IsOneWay(types);
}

SpeedKMpH const & BicycleModel::GetOffroadSpeed() const
{
  return bicycle_model::kSpeedOffroadKMpH;
}

// If one of feature types will be disabled for bicycles, features of this type will be simplified
// in generator. Look FeatureBuilder1::IsRoad() for more details.
// static
BicycleModel const & BicycleModel::AllLimitsInstance(RoutingOptions::OptionType mode)
{
  static BicycleModel const instance(mode, bicycle_model::AllAllowed(), bicycle_model::NormalPedestrianAndFootwaySpeed(mode));
  return instance;
}

// static
SpeedKMpH BicycleModel::DismountSpeed()
{
  return bicycle_model::kSpeedDismountKMpH;
}

BicycleModelFactory::BicycleModelFactory(CountryParentNameGetterFn const & countryParentNameGetterFn)
  : VehicleModelFactory(countryParentNameGetterFn)
{
  using namespace bicycle_model;

  //RoutingOptions const routingOptions = RoutingOptions::LoadCarOptionsFromSettings();
  //RoutingOptions::OptionType mode = routingOptions.GetBicycleMode();
  RoutingOptions::OptionType mode = RoutingOptions::CyclingGravel;

  // Names must be the same with country names from countries.txt
  m_models[""] = std::make_shared<BicycleModel>(mode, kDefaultOptions);

  m_models["Australia"] = std::make_shared<BicycleModel>(mode, AllAllowed(), NormalPedestrianAndFootwaySpeed(mode));
  m_models["Austria"] = std::make_shared<BicycleModel>(mode, NoTrunk(), DismountPathSpeed(mode));
  // Belarus law demands to use footways for bicycles where possible.
  m_models["Belarus"] = std::make_shared<BicycleModel>(mode, kDefaultOptions, PreferFootwaysToRoads(mode));
  m_models["Belgium"] = std::make_shared<BicycleModel>(mode, NoTrunk(), NormalPedestrianSpeed(mode));
  m_models["Brazil"] = std::make_shared<BicycleModel>(mode, AllAllowed());
  m_models["Denmark"] = std::make_shared<BicycleModel>(mode, NoTrunk());
  m_models["France"] = std::make_shared<BicycleModel>(mode, NoTrunk(), NormalPedestrianSpeed(mode));
  m_models["Finland"] = std::make_shared<BicycleModel>(mode, kDefaultOptions, NormalPedestrianSpeed(mode));
  m_models["Hungary"] = std::make_shared<BicycleModel>(mode, NoTrunk());
  m_models["Iceland"] = std::make_shared<BicycleModel>(mode, AllAllowed(), NormalPedestrianAndFootwaySpeed(mode));
  m_models["Ireland"] = std::make_shared<BicycleModel>(mode, AllAllowed());
  m_models["Italy"] = std::make_shared<BicycleModel>(mode, kDefaultOptions, NormalPedestrianSpeed(mode));
  m_models["Netherlands"] = std::make_shared<BicycleModel>(mode, NoTrunk());
  m_models["Norway"] = std::make_shared<BicycleModel>(mode, AllAllowed(), NormalPedestrianAndFootwaySpeed(mode));
  m_models["Oman"] = std::make_shared<BicycleModel>(mode, AllAllowed());
  m_models["Philippines"] = std::make_shared<BicycleModel>(mode, AllAllowed(), NormalPedestrianSpeed(mode));
  m_models["Poland"] = std::make_shared<BicycleModel>(mode, NoTrunk());
  m_models["Romania"] = std::make_shared<BicycleModel>(mode, AllAllowed());
  // Note. Despite the fact that according to
  // https://wiki.openstreetmap.org/wiki/OSM_tags_for_routing/Access-Restrictions passing through service and
  // living_street with a bicycle is prohibited it's allowed according to Russian traffic rules.
  m_models["Russian Federation"] = std::make_shared<BicycleModel>(mode, kDefaultOptions, NormalPedestrianAndFootwaySpeed(mode));
  m_models["Slovakia"] = std::make_shared<BicycleModel>(mode, NoTrunk());
  m_models["Spain"] = std::make_shared<BicycleModel>(mode, NoTrunk(), NormalPedestrianSpeed(mode));
  m_models["Sweden"] = std::make_shared<BicycleModel>(mode, kDefaultOptions, NormalPedestrianSpeed(mode));
  m_models["Switzerland"] = std::make_shared<BicycleModel>(mode, NoTrunk(), NormalPedestrianAndFootwaySpeed(mode));
  m_models["Ukraine"] = std::make_shared<BicycleModel>(mode, UkraineOptions());
  m_models["United Kingdom"] = std::make_shared<BicycleModel>(mode, AllAllowed());
  m_models["United States of America"] = std::make_shared<BicycleModel>(mode, AllAllowed(), NormalPedestrianSpeed(mode));
}
}  // namespace routing
