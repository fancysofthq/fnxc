#include <functional>
#include <memory>
#include <thread>
#include <variant>

#include "fnx/app/aot.hpp"
#include "fnx/lua/universe.hpp"
#include "fnx/onyx/parser.hpp"
#include "fnx/utils/logging.hpp"

namespace FNX {
namespace App {

// TODO: `require "a", "b"` is parallel, and `require "a"; require
// "b"` is sequential.

using namespace Utils::Logging;

AOT::AOT(
    std::optional<std::filesystem::path> output_path,
    LLIR::Linker::Mode mode,
    unsigned workers,
    std::optional<std::string> entry) :
    _thread_pool(workers) {}

void AOT::compile(
    const std::vector<std::filesystem::path> input_paths) {
  Onyx::Program program(&_thread_pool);
  program.require(input_paths);
  program.codegen();
}

} // namespace App
} // namespace FNX
