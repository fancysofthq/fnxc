#pragma once

#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <ostream>
#include <stack>
#include <stdint.h>
#include <unordered_map>
#include <variant>
#include <vector>

#include "../c/ast.hpp"
#include "../c/file.hpp"
#include "../lua/universe.hpp"
#include "../onyx/ast.hpp"
#include "../onyx/file.hpp"
#include "../onyx/program.hpp"
#include "../panic.hpp"

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
  /// An AOT compilation mode.
  enum class Mode {
    /// Build an executable file.
    Executable,

    /// Build a shared library.
    SharedLibrary,

    /// Build a static library.
    StaticLibrary,
  };

  /// @param input_paths Paths to input files. Multiple inputs would
  /// be compiled in parallel.
  ///
  /// @param output_path An optional output file path. Would be
  /// inferred for a single input, but required for multiple inputs.
  /// @param mode The compilation mode.
  ///
  /// @param workers Number of threads used by the compiler. FNX
  /// guarantees to never cross this threshold. @note It's not clear
  /// about libclang multi-threading.
  ///
  /// @param entry The name of the entry function, passed to the
  /// linker. Would be errornous for non-Executable mode.
  AOT(const std::optional<std::filesystem::path> output_path,
      Mode mode,
      std::optional<std::string> entry = std::nullopt);

  void compile(
      const std::vector<std::filesystem::path> input_paths,
      unsigned short workers);

private:
  struct CompilationUnit {
    /// An expected type of an input file.
    enum class Type {
      Onyx,
      C,
    };

    enum class State {
      Queued,
      InProgress,
      Compiled,
    };

    State state;

    const std::variant<
        std::shared_ptr<Onyx::File>,
        std::shared_ptr<C::File>>
        unit;

    const uint32_t id;

    CompilationUnit(std::filesystem::path, Type);
    const Type type();
    const std::filesystem::path path();
    friend std::ostream &operator<<(std::ostream &, const Type &);

  private:
    static std::atomic<uint32_t> id_incr;
  };

  std::unordered_map<
      std::shared_ptr<CompilationUnit>,
      CompilationUnit::State>
      _unit_states;

  std::stack<std::shared_ptr<CompilationUnit>> _units_queue;

  unsigned _in_progress_count;

  /// The queue mutex.
  std::mutex _mutex;

  /// The queue condition variable.
  std::condition_variable _condvar;

  /// The global panic container.
  /// Halts the compilation if not empty.
  std::optional<Panic> _panic;

  Onyx::Program _program;

  void _work();
  void _enqueue(std::filesystem::path, CompilationUnit::Type);

  /// It is executed from a working thread.
  void _compile(std::shared_ptr<CompilationUnit>);
};

} // namespace App
} // namespace FNX
