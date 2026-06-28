#include "testing/testing.hpp"

#include "routing/routing_options.hpp"

#include <cstdint>
#include <vector>

using namespace routing;

namespace
{
using OptionType = RoutingOptions::OptionType;

class RoutingOptionsTests
{
public:
  RoutingOptionsTests() { m_savedOptions = RoutingOptions::LoadOptionsFromSettings(VehicleType::Car); }

  ~RoutingOptionsTests() { RoutingOptions::SaveCarOptionsToSettings(m_savedOptions); }

private:
  RoutingOptions m_savedOptions;
};

RoutingOptions CreateOptions(std::vector<RoutingOptions::Option> const & include)
{
  RoutingOptions options;

  for (auto type : include)
    options.Add(type);

  return options;
}

void Checker(std::vector<RoutingOptions::Option> const & include)
{
  RoutingOptions options = CreateOptions(include);

  for (auto type : include)
    TEST(options.Has(type), ());

  auto max = static_cast<OptionType>(RoutingOptions::Max);
  for (uint8_t i = 1; i < max; i <<= 1)
  {
    bool hasInclude = false;
    auto type = static_cast<RoutingOptions::Option>(i);
    for (auto has : include)
      hasInclude |= (type == has);

    if (!hasInclude)
      TEST(!options.Has(static_cast<RoutingOptions::Option>(i)), ());
  }
}

UNIT_TEST(RoutingOptionTest)
{
  Checker({RoutingOptions::AvoidToll, RoutingOptions::AvoidMotorway, RoutingOptions::AvoidDirty});
  Checker({RoutingOptions::AvoidToll, RoutingOptions::AvoidDirty});

  Checker({RoutingOptions::AvoidToll, RoutingOptions::AvoidFerry, RoutingOptions::AvoidDirty});

  Checker({RoutingOptions::AvoidDirty});
  Checker({RoutingOptions::AvoidToll});
  Checker({RoutingOptions::AvoidDirty, RoutingOptions::AvoidMotorway});
  Checker({});
}

UNIT_CLASS_TEST(RoutingOptionsTests, GetSetTest)
{
  RoutingOptions options =
      CreateOptions({RoutingOptions::AvoidToll, RoutingOptions::AvoidMotorway, RoutingOptions::AvoidDirty});

  RoutingOptions::SaveCarOptionsToSettings(options);
  RoutingOptions fromSettings = RoutingOptions::LoadOptionsFromSettings(VehicleType::Car);

  TEST_EQUAL(options.GetOptions(), fromSettings.GetOptions(), ());
}
}  // namespace
