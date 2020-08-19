#pragma once

#include <mutex>
#include <optional>

#include "../../compiler/panic.hpp"

namespace Onyx {
namespace App {
namespace Shared {
// A byte code compiler. It compiles source code into byte code
// representing the source-level abstract syntax tree (SAST).
//
// The BC compiler can not be used as a standalone application;
// instead, it is a module included by other applications.
//
// This BC compiler implementation relies heavily on caching the byte
// code into `.nxbc` files.
class BC {
  stack<shared_ptr<Compiler::Unit>> _queued;
  unsigned int _in_progress; // The number of units in progress

  mutex _mutex;
  condition_variable _condvar;

protected:
  //   // FIXME: Make it constant.
  //   filesystem::path _root;

  optional<Compiler::Panic> _panic;

  // Enqueue a file for BC compilation.
  void enqueue(shared_ptr<Compiler::Unit>);

  // A working function for a thread.
  // NOTE: `enqueue` with the entry compilation unit shall be called
  // before any thread began working.
  void work();

private:
  void _compile(shared_ptr<Compiler::Unit>);
  void _wait(const vector<shared_ptr<Compiler::Unit>>);
  // virtual filesystem::path
  // _relative_path(shared_ptr<Compiler::Unit>);
};
} // namespace Shared
} // namespace App
} // namespace Onyx
