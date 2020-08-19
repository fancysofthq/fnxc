#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "../../../src/cpp/source/utils/bits.cpp"

TEST_CASE("testing `bits`") {
  uint8_t a = 'A';
  CHECK(bits(&a, sizeof(a)) == "01000001");

  int32_t b = 421769;
  CHECK(
      bits(&b, sizeof(b)) == "00000000 00000110 01101111 10001001");

  uint16_t c = 777;
  CHECK(bits(&c, sizeof(c), 0) == "0000001100001001");
}
