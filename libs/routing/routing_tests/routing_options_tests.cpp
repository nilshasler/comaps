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

  auto max = static_cast<OptionType>(RoutingOptions::Option::Max);
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
  Checker({RoutingOptions::Option::AvoidToll, RoutingOptions::Option::AvoidMotorway, RoutingOptions::Option::AvoidDirty});
  Checker({RoutingOptions::Option::AvoidToll, RoutingOptions::Option::AvoidDirty});

  Checker({RoutingOptions::Option::AvoidToll, RoutingOptions::Option::AvoidFerry, RoutingOptions::Option::AvoidDirty});

  Checker({RoutingOptions::Option::AvoidDirty});
  Checker({RoutingOptions::Option::AvoidToll});
  Checker({RoutingOptions::Option::AvoidDirty, RoutingOptions::Option::AvoidMotorway});
  Checker({});
}

UNIT_CLASS_TEST(RoutingOptionsTests, GetSetTest)
{
  RoutingOptions options =
      CreateOptions({RoutingOptions::Option::AvoidToll, RoutingOptions::Option::AvoidMotorway, RoutingOptions::Option::AvoidDirty});

  RoutingOptions::SaveCarOptionsToSettings(options);
  RoutingOptions fromSettings = RoutingOptions::LoadOptionsFromSettings(VehicleType::Car);

  TEST_EQUAL(options.GetOptions(), fromSettings.GetOptions(), ());
}
}  // namespace
