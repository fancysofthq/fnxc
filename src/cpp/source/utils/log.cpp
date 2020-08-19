#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

#include "../../header/utils/log.hpp"
#include "../../header/utils/null_stream.hpp"

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

static void output_header(const char level) {
  std::cerr << "[" << level << "][" << std::thread::id() << "][";
  output_time(std::cerr);
  std::cerr << "] ";
}

void fatal(const char *msg) { lfatal() << msg << std::endl; }
void error(const char *msg) { lerror() << msg << std::endl; }
void warn(const char *msg) { lwarn() << msg << std::endl; }
void info(const char *msg) { linfo() << msg << std::endl; }
void debug(const char *msg) { ldebug() << msg << std::endl; }
void trace(const char *msg) { ltrace() << msg << std::endl; }

void fatal(std::string msg) { fatal(msg.c_str()); }
void error(std::string msg) { error(msg.c_str()); }
void warn(std::string msg) { warn(msg.c_str()); }
void info(std::string msg) { info(msg.c_str()); }
void debug(std::string msg) { debug(msg.c_str()); }
void trace(std::string msg) { trace(msg.c_str()); }

std::ostream &lfatal() {
  const std::lock_guard lock(mutex);
  output_header('F');
  return std::cerr;
}

std::ostream &lerror() {
  if (verbosity >= Error) {
    const std::lock_guard lock(mutex);
    output_header('E');
    return std::cerr;
  } else
    return nullstream;
}

std::ostream &lwarn() {
  if (verbosity >= Warn) {
    const std::lock_guard lock(mutex);
    output_header('W');
    return std::cerr;
  } else
    return nullstream;
}

std::ostream &linfo() {
  if (verbosity >= Info) {
    const std::lock_guard lock(mutex);
    output_header('I');
    return std::cerr;
  } else
    return nullstream;
}

std::ostream &ldebug() {
  if (verbosity >= Debug) {
    const std::lock_guard lock(mutex);
    output_header('D');
    return std::cerr;
  } else
    return nullstream;
}

std::ostream &ltrace() {
  if (verbosity >= Trace) {
    const std::lock_guard lock(mutex);
    output_header('T');
    return std::cerr;
  } else
    return nullstream;
}
