#pragma once
#include "../../compile/panic.hpp"
#include <mutex>
#include <optional>

namespace Onyx {
namespace App {
namespace Shared {
// The canonical Byte-Code compiler implementation.
// It compiles source code into byte code
// (a binary source AST representation).
//
// The BC compiler can not be used as a standalone application;
// instead, it is a module included by other applications.
//
// This BC compiler implementation relies heavily
// on caching the bytecode into `.nxbc` files.
class BC {
  stack<shared_ptr<Compile::Unit>> _queued;
  unsigned int _in_progress; // The number of units in progress

  mutex _mutex;
  condition_variable _monitor;

protected:
  //   // FIXME: Make it constant.
  //   filesystem::path _root;

  optional<Compile::Panic> _panic;

  // Enqueue a file for BC compilation.
  void enqueue(shared_ptr<Compile::Unit>);

  // A working function for a thread.
  // NOTE: `enqueue` with the entry
  // compilation unit should be called
  // before any thread began working.
  void work();

private:
  void _compile(shared_ptr<Compile::Unit>);
  void _wait(const vector<shared_ptr<Compile::Unit>>);
  // virtual filesystem::path
  // _relative_path(shared_ptr<Compile::Unit>);
};
} // namespace Shared
} // namespace App
} // namespace Onyx
