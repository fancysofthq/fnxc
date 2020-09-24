#pragma once

#include <filesystem>
#include <istream>
#include <memory>
#include <optional>
#include <ostream>
#include <vector>

namespace FNX {
namespace App {
/**
 * @brief The Ahead-Of-Time compiler application.
 *
 * It builds the program into a single executable or library file.
 *
 * @note It does not support building freestanding object files.
 */
class AOT {
public:
  enum Mode {
    // Build an executable file.
    Executable,

    // Build a shared library.
    SharedLibrary,

    // Build a static library.
    StaticLibrary,
  };

  /**
   * @brief Compile a program
   *
   * @param input Input translation units.
   * Multiple inputs would share the same AST
   *
   * @param output  Output stream
   * @param mode    Compilation mode
   * @param workers Number of threads
   *
   * @param entry The name of the entry function.
   * Ignored for non-executables
   */
  void compile(
      const std::vector<std::shared_ptr<Onyx::Unit>> inputs,
      std::shared_ptr<std::ostream> output,
      Mode mode,
      unsigned short workers,
      std::optional<std::string> entry = std::nullopt);

private:
  void _enqueue(std::shared_ptr<Onyx::Unit> unit);
};
} // namespace App
} // namespace FNX
