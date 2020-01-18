#include "./shared/bc.hpp"

namespace Onyx {
namespace App {
// The canonical Just-In-Time compiler implementation.
// It lazily evaluates its input.
// The `--hot` option enables hot module reloading.
class JIT : Shared::BC {
public:
  JIT(filesystem::path input, bool hot, unsigned char workers);

  // Run the JIT compilation.
  void run();
};
} // namespace App
} // namespace Onyx
