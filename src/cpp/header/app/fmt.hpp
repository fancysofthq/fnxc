#include "./shared/bc.hpp"
#include <iostream>

namespace Onyx {
namespace App {
// The canonical formatter application.
// It formats an input file atomically.
//
// ```
// onyxc format main.nx -o main.formatted.nx
// onyxc format main.nx --rewrite
// ```
class FMT : Shared::BC {
public:
  FMT(filesystem::path input,
      filesystem::path output,
      unsigned char workers);

  // Format a file, returning a stream
  // to read the formatted source code from.
  istream *format();
};
} // namespace App
} // namespace Onyx
