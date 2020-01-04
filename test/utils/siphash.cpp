#include <cassert>

#include "../../src/utils/siphash.hpp"

void test(
    const uint64_t expected,
    const char key[16],
    const char input[],
    const int c,
    const int d) {
  SipHash hash(key, c, d);
  hash.update(input);
  assert(expected == hash.digest());
}

int main() {
  const char *key = "0123456789ABCDEF";
  const char *input = "hello";

  test(4402678656023170274, key, input, 2, 4);
  test(14986662229302055855UL, key, input, 4, 8);

  return 0;
}
