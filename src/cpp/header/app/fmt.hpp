#include "./shared/bc.hpp"
#include <iostream>

namespace Onyx {
namespace App {
// An Onyx source code formatter application.
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
