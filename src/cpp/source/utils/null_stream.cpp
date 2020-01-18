#include "../../header/utils/null_stream.hpp"

NullStream::NullStream() : std::ostream(nullptr) {}
NullStream::NullStream(const NullStream &) : std::ostream(nullptr) {}

template <class T>
const NullStream &operator<<(NullStream &&os, const T &value) {
  return os;
}
