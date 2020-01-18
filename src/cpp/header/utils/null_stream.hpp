#include <iostream>

// An output stream inspired by dev/null.
class NullStream : public std::ostream {
public:
  NullStream();
  NullStream(const NullStream &);
};

template <class T>
const NullStream &operator<<(NullStream &&os, const T &value);
