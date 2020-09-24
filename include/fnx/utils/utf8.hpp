#include <cinttypes>
#include <cstddef>
#include <stdexcept>
#include <stdint.h>

namespace FNX {
namespace Utils {
namespace UTF8 {

struct Error : std::logic_error {
  Error(const char *msg) : std::logic_error(msg) {}
};

/// Return the amount of bytes needed to encode a code point.
///
/// @code{.cpp}
///   CHECK(UTF8::codepoint_byte_size(246) == 2);
/// @endcode
size_t code_point_byte_size(char32_t code_point);

/// Return a code point size read from a potentially leading (the
/// first) code unit.
///
/// @code{.cpp}
///   CHECK(UTF8::size_from_leading_byte(195u) == 2);
/// @endcode
size_t size_from_leading_byte(char8_t ch);

/// Return a **thread-local** pointer to bytes representing given
/// code point.
///
/// @code{.cpp}
///   CHECK(std::string((char *)UTF8::to_codeunits(246)) == "ö");
/// @endcode
char8_t *to_code_units(char32_t code_point);

/// Return a 32-bit code point integer decoded from code units.
///
/// @code{.cpp}
///   CHECK(UTF8::to_codepoint(u8"ö") == 246);
/// @endcode
char32_t to_code_point(const char8_t *code_units);

} // namespace UTF8
} // namespace Utils
} // namespace FNX
