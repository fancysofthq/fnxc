#pragma once

#include "../llir/linker.hpp"
#include "../onyx/program.hpp"

namespace FNX {
namespace App {

/// The Ahead-Of-Time compiler application.
///
/// It builds the program into a single executable or library file.
///
/// @details
/// An AOT instance contains a Program instance.
///
/// @note It does not support building freestanding object files.
class AOT {
public:
  AOT(Utils::ThreadPool *);

  /// @param input_paths Paths to input files. Multiple inputs would
  /// be compiled in parallel.
  ///
  /// @param output_path An optional output file path. Would be
  /// inferred for a single input, but required for multiple inputs.
  ///
  /// @param mode The compilation mode.
  ///
  /// @param workers Number of threads used by the compiler. FNX
  /// guarantees to never cross this threshold. @note It's not clear
  /// about libclang multi-threading.
  ///
  /// @param entry The name of the entry function, passed to the
  /// linker. Would be errornous for non-Executable mode.

  void compile(
      const std::vector<std::filesystem::path> input_paths,
      std::optional<std::filesystem::path> output_path,
      LLIR::Linker::Mode mode,
      unsigned workers,
      std::optional<std::string> entry = std::nullopt);

private:
  Utils::ThreadPool *_thread_pool;
};

} // namespace App
} // namespace FNX
