#include "../../../header/app/shared/bc.hpp"
#include "../../../header/utils/log.hpp"
#include <fstream>
#include <sstream>

namespace Onyx {
namespace App {
namespace Shared {
void BC::enqueue(shared_ptr<Compile::Unit> unit) {
  ltrace() << "[BC] Locking to enqueue " << unit->path << "...";
  lock_guard<mutex> lock(_mutex);

  _queued.push(unit);
  ldebug() << "[BC] Enqueued " << unit->path;

  _monitor.notify_all();
}

void BC::work() {
  ldebug() << "[BC] Start working...";

  while (true) {
    ltrace() << "[BC] Acquiring a working lock...";
    unique_lock<mutex> lock(_mutex);

    if (_panic) {
      ltrace() << "[BC] Breaking work because panic is set";
      break;
    }

    if (_queued.size() > 0) {
      ltrace() << "[BC] Popping an enqueued unit";
      const auto unit = _queued.top();
      _queued.pop();
      ltrace() << "[BC] Popped " << unit->path;

      if (unit->state == Compile::Unit::Queued) {
        ltrace() << "[BC] Setting " << unit->path
                 << " state to `being compiled`";

        _in_progress += 1;
        unit->state = Compile::Unit::BeingCompiled;
        lock.unlock();

        try {
          _compile(unit);
        } catch (Compile::Panic &p) {
          ltrace() << "[BC] Panicked, acquiring a lock...";
          unique_lock<mutex> lock(_mutex);

          _panic = make_unique<Compile::Panic>(p);
          lock.unlock();
          _monitor.notify_all();

          ltrace() << "[BC] Breaking the loop because of the panic";
          break;
        }
      } else {
        ldebug() << unit->path << " does not have `queued` state";
      }
    } else if (_in_progress > 0) {
      ltrace() << "[BC] Waiting for a queue notification...";
      _monitor.wait(lock);
      ltrace() << "[BC] Received a queue notification";
    } else {
      ltrace() << "[BC] Queues are empty, breaking the loop";
      break;
    }
  }

  ldebug() << "[BC] The worker is done";
}

void BC::_compile(shared_ptr<Compile::Unit> unit) {
  auto lexer = Lexer(unit);
  auto parser = Parser(&lexer);

  try {
    ltrace() << "[BC] Parsing the unit requirements";
    auto reqs = parser.requirements();

    if (!reqs.empty()) {
      ltrace() << "[BC] Have " << reqs.size() << " requirements";
      vector<shared_ptr<Compile::Unit>> to_compile;

      for (auto req : reqs) {
        if (req.is_import)
          ltrace() << "[BC] Would compile imported " << req.path;
        else
          ltrace() << "[BC] Would compile required " << req.path;

        filesystem::path absolute_path;

        if (req.path.is_relative())
          absolute_path = unit->path.parent_path() / req.path;
        else
          absolute_path = req.path;

        to_compile.push_back(make_shared<Compile::Unit>(
            req.is_import, absolute_path, unit));
      }

      if (!to_compile.empty())
        _wait(to_compile);
      else
        ltrace() << "[BC] No units to wait for compilation";
    }

    while (true) {
      ltrace() << "[BC] Parsing next node";

      if (!parser.next()) {
        ltrace() << "[BC] Parser returned nullptr, breaking the loop";
        break;
      }
    }

    ltrace() << "[BC] Acquiring an after-compilation lock... ";
    lock_guard<mutex> lock(_mutex);
    ltrace() << "[BC] Acquired an after-compilation lock";

    ldebug() << "[BC] Successfully compiled " << unit->path;
    unit->state = Compile::Unit::Compiled;

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

void BC::_wait(const vector<shared_ptr<Compile::Unit>> units) {
  auto size = units.size();

  // Build a string list of required paths
  // for convenient tracing
  stringstream ss;
  bool first = true;
  for (auto unit : units) {
    if (!first)
      ss << ", ";
    else
      first = false;

    ss << unit->path;
  }

  ltrace() << "[BC] Waiting for " << size
           << " units to compile: " << ss.str();

  for (auto unit : units) {
    while (true) {
      ltrace() << "[BC] Acquiring a lock at wait()...";
      unique_lock<mutex> lock(_mutex);

      if (_panic) {
        ltrace() << "[BC] Returning from wait() because of a panic";
        return;
      } else if (unit->state == Compile::Unit::Compiled) {
        ltrace() << "[BC] Done waiting for " << unit->path;
        break;
      } else {
        if (_queued.size() > 0) {
          ltrace() << "[BC] Popping an enqueued unit while waiting";
          const auto next_unit = _queued.top();
          _queued.pop();

          if (next_unit->state == Compile::Unit::Queued) {
            next_unit->state = Compile::Unit::BeingCompiled;
            ltrace() << "[BC] Got unit " << next_unit->path
                     << " for compilation while waiting";
            lock.unlock();
            _compile(next_unit);
          } else {
            stringstream ss;
            ss << "BUG: Popped unit " << next_unit->path
               << " does not have `queued` state";
            throw std::exception(ss.str().c_str());
          }
        } else {
          ltrace() << "[BC] No more enqueued units. "
                   << "Waiting for a notification...";
          _monitor.wait(lock);
        }
      }
    }
  }
}
} // namespace Shared
} // namespace App
} // namespace Onyx
