#include "./shared/bc.hpp"

namespace Onyx {
namespace App {
// The Just-In-Time compiler application.
class JIT : Shared::BC {
public:
  JIT(filesystem::path input, bool hot, unsigned char workers);

  // Run the JIT compilation.
  void run();
};
} // namespace App
} // namespace Onyx
