#include <cstdint>
#include <string>

namespace FNX {
namespace Utils {
namespace FNV1a {

uint32_t hash32(const void *, const uint32_t length);
uint64_t hash64(const void *, const uint64_t length);
uint32_t hash32(const std::string);
uint64_t hash64(const std::string);

} // namespace FNV1a
} // namespace Utils
} // namespace FNX
