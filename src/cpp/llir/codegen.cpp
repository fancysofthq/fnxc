#include "fnx/llir/codegen.hpp"

namespace FNX {
namespace LLIR {

std::vector<Module> Codegen::generate(
    Onyx::Program *program,
    std::optional<std::filesystem::path> cache_dir,
    std::optional<std::tuple<std::filesystem::path, bool>> output) {
  // Iterate through the exported functions, in order of declaration.
  //
  // Once a type is mentioned, look up for its specialization in some
  // map. If not found, check if the file's cache contains bytecode
  // for the specialization. If so, directly write  create a new LLVM
  // type in the module associated with the file which contains the
  // complete implementation, or load the type from the file's cache.
}

} // namespace LLIR
} // namespace FNX
