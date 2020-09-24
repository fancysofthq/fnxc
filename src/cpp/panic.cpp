#include "../../include/fnx/panic.hpp"

namespace FNX {

Panic::Panic(Location loc, std::string msg) : std::logic_error(msg) {
  backtrace.push(loc);
}

} // namespace FNX
