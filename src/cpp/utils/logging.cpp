#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include "../../../include/fnx/utils/logging.hpp"
#include "../../../include/fnx/utils/null_stream.hpp"

namespace FNX {
namespace Utils {
namespace Logging {

static std::mutex mutex;
static NullStream nullstream;

// Outputs time to a stream in "%H:%M:%S.%ms" format.
static void output_time(std::ostream &s) {
  using namespace std::chrono;

  auto now = system_clock::now();
  auto ms =
      duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
  auto a = system_clock::to_time_t(now);
  std::tm b = *std::localtime(&a);

  s << std::put_time(&b, "%H:%M:%S");
  s << '.' << std::setfill('0') << std::setw(3) << ms.count();
}

static void
output_header(const char level, std::string context = NULL) {
  std::cerr << "[" << level << "][" << std::thread::id() << "][";

  output_time(std::cerr);
  std::cerr << "] ";

  if (!(context.empty()))
    std::cerr << "[" << context << "] ";
}

#define IMPL_LOG(LEVEL, VERBOSITY, LEVEL_ID)                        \
  std::ostream &LEVEL(std::string context) {                        \
    if (verbosity >= Verbosity::VERBOSITY) {                        \
      output_header(LEVEL_ID, context);                             \
      return std::cerr;                                             \
    } else                                                          \
      return nullstream;                                            \
  }

IMPL_LOG(fatal, Fatal, 'F')
IMPL_LOG(error, Error, 'E')
IMPL_LOG(warn, Warn, 'W')
IMPL_LOG(info, Info, 'I')
IMPL_LOG(debug, Debug, 'D')
IMPL_LOG(trace, Trace, 'T')

#undef IMPL_LOG

} // namespace Logging
} // namespace Utils
} // namespace FNX
