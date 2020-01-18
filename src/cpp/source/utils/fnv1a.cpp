#include "../../header/utils/fnv1a.hpp"

uint32_t FNV1a::hash32(const void *input, const uint32_t length) {
  const char *data = (char *)input;

  uint32_t hash = 0x811c9dc5;
  uint32_t prime = 0x1000193;

  for (int i = 0; i < length; ++i) {
    uint8_t value = data[i];
    hash = hash ^ value;
    hash *= prime;
  }

  return hash;
}

uint32_t FNV1a::hash32(const std::string input) {
  return hash32(input.c_str(), input.size());
}

uint64_t FNV1a::hash64(const void *input, const uint64_t length) {
  const char *data = (char *)input;

  uint64_t hash = 0xcbf29ce484222325;
  uint64_t prime = 0x100000001b3;

  for (int i = 0; i < length; ++i) {
    uint8_t value = data[i];
    hash = hash ^ value;
    hash *= prime;
  }

  return hash;
}

uint64_t FNV1a::hash64(const std::string input) {
  return hash64(input.c_str(), input.size());
}
