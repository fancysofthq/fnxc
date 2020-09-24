#include <string>

namespace FNX {
namespace Utils {
namespace String {

std::string format(const char *...);

/// A wrapper around std::sto* functions family.
template <class N, class S> N parse(S string, unsigned base);

template <> int parse(std::u32string string, unsigned base);
template <> int parse(char32_t string, unsigned base);

template <> int parse(std::u8string string, unsigned base);
template <> int parse(char8_t string, unsigned base);

} // namespace String
} // namespace Utils
} // namespace FNX
