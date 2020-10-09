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

void AOT::_enqueue(
    std::filesystem::path path, CompilationUnit::Type type) {
  _units_queue.push(std::make_shared<CompilationUnit>(path, type));
  trace(__func__) << "Enqueued " << type << " file at " << path;
}

void AOT::compile(
    const std::vector<std::filesystem::path> input_paths,
    unsigned short workers_count) {
  for (auto path : input_paths) {
    _enqueue(path, CompilationUnit::Type::Onyx);
  }

  std::vector<std::thread> workers;
  // void (Instance::*_work)();

  // Spawn workers for compilation
  for (int i = 0; i < workers_count; i++) {
    trace(__func__) << "Spawning a worker...";
    auto fun = std::bind(&AOT::_work, this);
    workers.push_back(std::thread(fun));
    trace(__func__) << "Spawned worker @" << workers.back().get_id();
  }

  // "Workers of the world — unite"!
  for (auto &worker : workers) {
    worker.join();
    trace(__func__) << "Joined worker @" << worker.get_id();
  }

  if (_panic.has_value()) {
    // Something went wrong during compilation
    trace(__func__) << "Throwing panic";
    throw _panic.value();
  }

  trace(__func__) << "Successfully compiled the program";
}

void AOT::_work() {
  while (true) {
    std::unique_lock lock(_mutex);

    if (_panic) {
      trace(__func__) << "Breaking the loop because `_panic` is set";
      break;
    }

    if (_units_queue.size() > 0) {
      auto unit = _units_queue.top();
      _units_queue.pop();

      trace(__func__) << "Popped " << unit->type() << " file at "
                      << unit->path();

      if (unit->state == CompilationUnit::State::Queued) {
        _in_progress_count++;
        unit->state = CompilationUnit::State::InProgress;

        // Unlock early, as now we're now working with this very
        // unit, which is thread-safe.
        lock.unlock();

        try {
          _compile(unit);
        } catch (Panic p) {
          // We need a lock to set the `_panic` value.
          //

          trace(__func__) << "Panicked, acquiring a lock...";
          std::unique_lock lock(_mutex);
          trace(__func__) << "Acquired the lock";

          _panic = p;
          trace(__func__) << "Set `_panic`";

          lock.unlock();
          trace(__func__) << "Released the lock";

          // Notify all the sleeping threads because of the panic
          _condvar.notify_all();
          trace(__func__) << "Notified all threads";

          trace(__func__) << "Breaking the loop because of panic";
          break;
        }

        trace(__func__) << "Successfully compiled " << unit->type()
                        << " file at " << unit->path();
      } else {
        // The unit may already be or being compiled
        trace() << "File at " << unit->path()
                << "does not have `queued` state";
      }
    } else if (_in_progress_count > 0) {
      // There is still compilation in progress, thus shall wait for
      // new units to compile...
      trace(__func__) << "Waiting for notification...";
      _condvar.wait(lock);
      trace(__func__) << "Received a notification";
    } else {
      trace() << "Breaking the loop because the queue is empty";
      break;
    }
  }

  trace(__func__) << "Done";
}

void AOT::_compile(std::shared_ptr<CompilationUnit> unit) {
  switch (unit->type()) {
  case CompilationUnit::Type::Onyx: {
    auto file = std::get<std::shared_ptr<Onyx::File>>(unit->unit);
    auto parser = Onyx::Parser(file->source_stream());
    auto cst = parser.parse();
  }

  case CompilationUnit::Type::C: {
    // TODO: Use C builder?
  }
  }
}

} // namespace App
} // namespace FNX
