#pragma once

#include <iostream>
#include <string>

namespace FNX {
namespace Utils {
namespace Logging {

enum struct Verbosity {
  Fatal,
  Error,
  Warn,
  Info,
  Debug,
  Trace,
};

Verbosity verbosity;

#define DEF_LOGGING(LEVEL)                                          \
  std::ostream &LEVEL(std::string context = NULL);

DEF_LOGGING(fatal)
DEF_LOGGING(error)
DEF_LOGGING(warn)
DEF_LOGGING(info)
DEF_LOGGING(debug)
DEF_LOGGING(trace)

#undef DEF_LOGGING

} // namespace Logging
} // namespace Utils
} // namespace FNX
