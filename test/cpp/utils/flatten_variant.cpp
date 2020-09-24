#include <type_traits>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "../../../include/fnx/utils/flatten_variant.hpp"

using namespace FNX::Utils;

TEST_CASE("`FNX::Utils::flatten_variant_t`") {
  class Klass {};

  using nested = std::variant<
      int,
      double,
      std::variant<std::string, int>,
      std::variant<Klass>>;

  using flattened =
      std::variant<int, double, std::string, int, Klass>;

  static_assert(
      std::is_same<flatten_variant_t<nested>, flattened>{},
      "FNX::Utils::flatten_variant_t failed the test");
}
