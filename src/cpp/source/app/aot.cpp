#include <functional>

#include "../../header/app/aot.hpp"
#include "../../header/utils/log.hpp"

namespace Onyx {
namespace App {
AOT::AOT(
    // filesystem::path root,
    filesystem::path input,
    filesystem::path output,
    bool lib,
    unsigned short workers) :
    _entry(make_shared<Compiler::Unit>(
        Compiler::Unit(false, input, nullptr))),
    _output(output),
    _is_lib(lib),
    _workers(workers) {
  // _root = root;
}

void AOT::compile() {
  enqueue(_entry);

  vector<thread> workers;
  // void (Instance::*_work)();

  // Spawn workers for BC compilation
  for (int i = 0; i < _workers; i++) {
    ltrace() << "Starting worker #" << (i + 1);
    auto fun = bind(&AOT::work, this);
    workers.push_back(thread(fun));
    ltrace() << "Worker #" << (i + 1) << " has ID @"
             << workers.back().get_id();
  }

  // "Workers of the world — unite"!
  // For AST building, we won't need multiple workers.
  for (auto &worker : workers) {
    worker.join();
    ltrace() << "Joined worker @" << worker.get_id();
  }

  if (_panic.has_value()) {
    // Something went wrong during BC compilation
    ltrace() << "Throwing a BC panic";
    throw _panic.value();
  } else
    ltrace() << "The BC compiler did not panic";
}
} // namespace App
} // namespace Onyx
