#include <condition_variable>
#include <experimental/filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
#include <stack>
#include <thread>
#include <unordered_map>
#include <vector>

#include "./compiler.hpp"
#include "./compiler/lexer.hpp"
#include "./compiler/parser.hpp"
#include "./compiler/token.hpp"
#include "./utils/log.hpp"

namespace Onyx {
namespace Compiler {

Unit::Unit(const fs::path path, const shared_ptr<Unit> parent) :
    path(path),
    parent(parent) {}

Location::Location(
    const shared_ptr<Unit> unit,
    unsigned int brow,
    unsigned int bcol,
    unsigned int erow,
    unsigned int ecol) :
    unit(unit),
    brow(brow),
    bcol(bcol),
    erow(erow),
    ecol(ecol) {}

Location::Location(
    const shared_ptr<Unit> unit, unsigned int row, unsigned int col) :
    unit(unit),
    brow(row),
    bcol(col),
    erow(row),
    ecol(col) {}

Panic::Panic(Location location, string message) :
    message(message),
    std::logic_error(message) {
  backtrace.push(location);
}

Instance::Instance(fs::path entry, unsigned char workers_count) :
    _entry(make_shared<Unit>(Unit(entry, nullptr))),
    _workers_count(workers_count) {}

void Instance::compile_sast() {
  _enqueue(_entry);

  vector<thread> workers;
  // void (Instance::*_work)();

  // Spawn *workers_count* workers
  for (int i = 0; i < _workers_count; i++) {
    ltrace() << "Starting worker #" << (i + 1);
    auto fun = bind(&Instance::_work, this);
    workers.push_back(thread(fun));
    ltrace() << "Worker #" << (i + 1) << " has ID @"
             << workers.back().get_id();
  }

  int i = 0;

  // Workers of the world — unite!
  for (auto &worker : workers) {
    worker.join();
    ltrace() << "Joined worker @" << worker.get_id();
  }

  if (_panic)
    throw _panic.get();
}

void Instance::_enqueue(const shared_ptr<Unit> unit) {
  ltrace() << "Locking to enqueue " << unit->path << "...";
  lock_guard<mutex> lock(_mutex);

  _queued.push_back(unit);
  ldebug() << "Enqueued " << unit->path;

  _monitor.notify_all();
}

void Instance::_work_sast() {
  ldebug() << "Start working...";

  while (true) {
    ltrace() << "Acquiring a working lock...";
    unique_lock<mutex> lock(_mutex);

    if (_panic) {
      ltrace() << "Breaking work because panic is set";
      break;
    }

    if (_queued.size() > 0) {
      ltrace() << "Popping an enqueued unit";
      const auto unit = _queued.back();
      _queued.pop_back();
      ltrace() << "Popped " << unit->path;

      if (unit->state == Unit::Queued) {
        ltrace() << "Setting " << unit->path
                 << " state to `being compiled`";

        _being_compiled_counter += 1;
        unit->state = Unit::BeingCompiled;
        lock.unlock();

        try {
          _compile_unit(unit);
        } catch (Panic &p) {
          ltrace() << "Panicked, acquiring a lock...";
          unique_lock<mutex> lock(_mutex);

          _panic = make_unique<Panic>(p);
          lock.unlock();
          _monitor.notify_all();

          ltrace() << "Breaking the loop because of the panic";
          break;
        }
      } else {
        ldebug() << unit->path << " does not have `queued` state";
      }
    } else if (_being_compiled_counter > 0) {
      ltrace() << "Waiting for a queue notification...";
      _monitor.wait(lock);
      ltrace() << "Received a queue notification";
    } else {
      ltrace() << "Queues are empty, breaking the loop";
      break;
    }
  }

  ldebug() << "The worker is done";
}

static void _compile_file(const fs::path path) {
  ldebug() << "Opening file " << path << " for compilation";

  if (!fs::exists(path)) {
    ltrace() << "File does not exist, panicking";
    throw Panic(
        Location(),
        "Could not open \"" + path.generic_string() + '"');
  }

  ifstream file;

  try {
    file = ifstream(path);
  } catch (fs::filesystem_error &err) {
    ltrace() << "Caught filesystem error (" << err.what()
             << "), panicking";
    throw Panic(Location(), err.what());
  }

  ldebug() << "Opened the file, begin compiling";

  auto tokens_stack = _tokens.at(path);
  auto lexer = Lexer(&file, &tokens_stack);

  auto sast = _sasts.at(path);
  auto parser = Parser(&lexer, sast);

  try {
    ltrace() << "Parsing the file requirements";
    auto reqs = parser.requirements();

    if (!reqs.empty()) {
      ltrace() << "Have " << reqs.size() << " requirements";
      stack<fs::path> to_compile;

      for (auto req : reqs) {
        fs::path resulting_path;

        if (req.path.is_relative())
          resulting_path = path.parent_path() / req.path;
        else
          resulting_path = req.path;

        if (req.is_import) {
          ltrace() << "Would `import` " << resulting_path;
          _imports.insert(resulting_path);
        } else {
          ltrace() << "Would `require` " << resulting_path;
          to_compile.push(resulting_path);
        }
      }

      if (!to_compile.empty())
        _wait(&to_compile);
      else
        ltrace() << "No files `require`d to compile";
    }

    while (true) {
      ltrace() << "Parsing next node";

      if (!parser.next()) {
        ltrace() << "Parser returned nullptr, breaking the loop";
        break;
      }
    }

    ltrace() << "Acquiring the lock after successfull compilation...";
    lock_guard<mutex> lock(_mutex);
    ltrace() << "Acquired the after-compilation lock";

    ltrace() << "Removing the path from the being compiled list";
    _being_compiled.erase(path);

    ltrace() << "Inserting the path into the compiled list";
    _compiled.insert(path);

    ldebug() << "Compiled " << path;
    _monitor.notify_all();
  } catch (Lexer::Error err) {
    throw Error(Location(path, err.location), err.message);
  } catch (Parser::Error err) {
    throw Error(Location(path, err.token->location), err.reason);
  } catch (Error err) {
    // TODO: Exact position of the require path in current file
    const auto back =
        Error(Location(path), L"required from this file");
    err.backtrace.push_back(back);
    throw err;
  }
}

static void _wait(const vector<fs::path> *required) {
  auto size = required->size();

  // Build a string list of required paths
  // for convenient tracing
  stringstream ss;
  bool first = true;
  for (auto path : *required) {
    if (!first)
      ss << ", ";
    else
      first = false;

    ss << path;
  }

  ltrace() << "Waiting for " << size
           << " required paths to compile: " << ss.str();

  for (auto path : *required) {
    while (true) {
      ltrace() << "Acquiring a lock at wait()...";
      unique_lock<mutex> lock(_mutex);

      if (_panic) {
        ltrace() << "Returning from wait() because of a panic";
        return;
      } else if (_compiled.count(path)) {
        ltrace() << "Done waiting for " << path;
        break;
      } else {
        if (_queued.size() > 0) {
          ltrace() << "Popping an enqueued path while waiting";
          const auto next_path = _queued.back();
          _queued.pop_back();

          if (!_being_compiled.count(next_path)) {
            _being_compiled.insert(next_path);
            ltrace() << "Got " << path
                     << " for compilation while waiting";
            lock.unlock();
            _compile_file(next_path);
            break;
          } else {
            ltrace() << "Popped file " << path
                     << " is already being compiled. "
                     << "Waiting for a notification...";
            _monitor.wait(lock);
          }
        } else {
          ltrace() << "No more enqueued files. "
                   << "Waiting for a notification...";
          _monitor.wait(lock);
        }
      }
    }
  }
}
} // namespace Compiler
}; // namespace Onyx
