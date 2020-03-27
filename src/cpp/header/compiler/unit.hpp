#pragma once

#include <filesystem>
#include <stack>

using namespace std;

namespace Onyx {
namespace Compiler {

namespace AST {
struct Root;
}

// namespace Token {
// struct Base;
// }

// A compilation unit.
struct Unit {
  enum State { Queued, BeingCompiled, Compiled };
  State state;

  // The SAST root for the unit. It may be empty
  // if the unit is skipped due to caching etc.
  shared_ptr<AST::Root> sast;

  // // The container to store the unit's tokens.
  // // This includes both tokens from source files and evaluated
  // from
  // // macros. Token preservation is needed to properly output them.
  // stack<shared_ptr<Token::Base>> tokens;

  // An absolute file path.
  const filesystem::path path;

  const bool is_import;
  const shared_ptr<Unit> parent;

  Unit(bool is_import, filesystem::path, shared_ptr<Unit> parent) :
      is_import(is_import), parent(parent) {}

  // filesystem::path relative_path(filesystem::path root);
};
} // namespace Compiler
} // namespace Onyx
