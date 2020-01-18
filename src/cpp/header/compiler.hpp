#pragma once

#include "./compiler/sast.hpp"
#include <experimental/filesystem>
#include <mutex>
#include <stack>

using namespace std;
namespace fs = experimental::filesystem;

namespace Onyx {
namespace Compiler {
// A compilation unit.
struct Unit {
  enum State { Queued, BeingCompiled, Compiled };
  State state;

  // The SAST root for the unit. It may be empty
  // if the unit is skipped due to caching etc.
  shared_ptr<SAST::Root> sast;

  // The container to store the unit's tokens.
  // This includes both tokens from source files and evaluated from
  // macros. Token preservation is needed to properly output them.
  stack<shared_ptr<Token::Base>> tokens;

  const fs::path path;
  const shared_ptr<Unit> parent;

  Unit(const fs::path, const shared_ptr<Unit> parent);
};

// A location in a compilation unit.
// It may optionally be spanning, i.e. have end row and column.
struct Location {
  const shared_ptr<Unit> unit;

  const unsigned int brow;
  const unsigned int bcol;
  const unsigned int erow;
  const unsigned int ecol;

  Location(
      const shared_ptr<Unit>,
      unsigned int brow,
      unsigned int bcol,
      unsigned int erow,
      unsigned int ecol);

  Location(
      const shared_ptr<Unit>, unsigned int row, unsigned int col);
};

// A compiler panic occured due to a source program error.
struct Panic : std::logic_error {
  const string message;
  stack<Location> backtrace;

  Panic(Location, string message);
};

// A compiler instance.
class Instance {
  const shared_ptr<Unit> _entry;
  const unsigned char _workers_count;

  // Source files compilation tracking
  //

  vector<shared_ptr<Unit>> _queued;
  unsigned int _being_compiled_counter;
  // set<shared_ptr<Unit>> _being_compiled;
  // set<shared_ptr<Unit>> _compiled;

  set<shared_ptr<Unit>> _imports;

  // Syncronization primitives
  //

  mutex _mutex;
  condition_variable _monitor;

  // A compiler panic occured in the multi-threaded environment.
  unique_ptr<Panic> _panic;

public:
  Instance(fs::path entry, unsigned char workers_count);

  void compile_sast();
  void compile_ast();
  void codegen();

private:
  // The entry function for a compiler worker.
  void _work_sast();

  // Enqueue a unit for (re)compilation.
  void _enqueue(const shared_ptr<Unit>);

  // Compile a unit in a blocking manner.
  void _compile_unit(const shared_ptr<Unit>);

  // Wait until **all** given units compile.
  void _wait(const stack<shared_ptr<Unit>> *);
};
}; // namespace Compiler
} // namespace Onyx
