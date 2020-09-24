#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "../../../src/cpp/utils/fnv1a.cpp"

using namespace FNX::Utils;

TEST_CASE("testing FNV1a implementations") {
  CHECK(FNV1a::hash32("hello", 6) == 43209009);
  CHECK(FNV1a::hash64("hello", 6) == 12230803299529341361uL);
}
