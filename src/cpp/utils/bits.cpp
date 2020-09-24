#include <sstream>

#include "../../../include/fnx/utils/bits.hpp"

std::string
FNX::Utils::bits(const void *obj, size_t size, char separator) {
  std::stringstream stream;
  unsigned char *b = (unsigned char *)obj;
  unsigned char byte;
  int i, j;

  for (i = size - 1; i >= 0; i--) {
    for (j = 7; j >= 0; j--) {
      byte = (b[i] >> j) & 1;
      // printf("%u", byte);
      stream << +byte;
    }

    if (separator && i != 0)
      stream << separator;
  }

  return stream.str();
}
