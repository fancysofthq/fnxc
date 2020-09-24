#include "../../../include/fnx/utils/null_stream.hpp"

namespace FNX {
namespace Utils {

NullStream::NullStream() : std::ostream(nullptr) {}
NullStream::NullStream(const NullStream &) : std::ostream(nullptr) {}

template <class T>
const NullStream &operator<<(NullStream &&os, const T &value) {
  return os;
}

} // namespace Utils
} // namespace FNX
