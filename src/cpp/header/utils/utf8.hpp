#include <cinttypes>
#include <stdexcept>

namespace UTF8 {
struct Error : std::logic_error {
  Error(const char *msg) : std::logic_error(msg) {}
};

// Return the amount of bytes needed to encode a codepoint.
size_t codepoint_byte_size(uint32_t codepoint);

// Return a codepoint size read from a
// potential leading (the first) codeunit.
//
// ```
// CHECK(UTF8::size_from_leading_byte(65) == "A");
// ```
size_t size_from_leading_byte(char ch);

// Return a pointer to chars representing given codepoint.
// NOTE: The pointer is thread-local, thus it must be copied
// as soon as possible to avoid data corruption.
//
// ```
// CHECK(std::string(UTF8::to_codeunits(65)) == "A");
// ```
char *to_codeunits(uint32_t codepoint);

// Return a 32-bit codepoint integer decoded from codeunits.
//
// ```
// CHECK(UTF8::to_codepoint("A") == 65);
// ```
uint32_t to_codepoint(const char *codeunits);
} // namespace UTF8
