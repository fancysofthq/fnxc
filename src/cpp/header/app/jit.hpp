#include "./shared/bc.hpp"

namespace Onyx {
namespace App {
// The canonical Just-In-Time compiler implementation.
// It lazily evaluates its input.
// The `--hot` option enables hot module reloading.
//
// Only `-O0`, `-O1` and `-O2` flags would work in JIT mode, as there
// is no need to worry about binary size. Lesser `-O` reduces the time
// required to pre-compile the code, while greater `-O` increases the
// compiled code performance.
class JIT : Shared::BC {
public:
  JIT(filesystem::path input, bool hot, unsigned char workers);

  // Run the JIT compilation.
  void run();
};
} // namespace App
} // namespace Onyx
