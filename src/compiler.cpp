#include <fstream>

#include "./compiler.hpp"
#include "./lexer.hpp"
#include "./parser.hpp"

namespace Onyx {
Compiler::Error::Error(Location location, wstring message) :
    location(location),
    message(message) {}

Compiler::Compiler(fs::path entry, ushort workers_count) {
  _top_level_namespace = make_unique<SAST::Namespace>();

  enqueue(entry);

  vector<thread> workers;
  void (Compiler::*work)(int);

  // Spawns *workers_count* workers
  for (int i = 0; i < workers_count; i++) {
    auto fun = bind(&Compiler::work, this);
    workers.push_back(thread(fun));
  }

  // Workers of the world — unite!
  for (int i = 0; i < workers_count; i++) {
    workers.back().join();
    workers.pop_back();
  }

  if (_error)
    throw *_error.get();
  else
    _top_level_namespace->dump(&wcerr);
}

void Compiler::enqueue(const fs::path path) {
  lock_guard<mutex> lock(_mutex);
  _queued.push_back(path);
  _monitor.notify_all();
}

void Compiler::compile_file(const fs::path path) {
  if (!fs::exists(path))
    throw Error(
        Location(), L"Could not open \"" + path.generic_wstring() + L'"');

  wifstream file;

  try {
    file = wifstream(path);
  } catch (fs::filesystem_error err) {
    throw Error(Location(), to_wstring(err.what()));
  }

  debug(string("Compiling ").append(path) + "...");

  // auto macro = _macros.at(this_thread::get_id()).get();
  auto lexer = Lexer(&file, nullptr);
  auto parser =
      Parser(&lexer, &_tokens.at(path), _top_level_namespace, &_sast_mutex);

  try {
    auto request = parser.parse();
    auto require_buffer = vector<fs::path>();

    while (request.has_value()) {
      auto is_import = request.value().is_import;
      auto paths = request.value().paths;

      if (!is_import) {
        for (int i = 0; i < paths.size(); i++) {
          const auto required_path = fs::path(paths.at(i));

          fs::path resulting_path;

          if (required_path.is_relative())
            resulting_path = path.parent_path() / required_path;
          else
            resulting_path = required_path;

          enqueue(resulting_path);
          require_buffer.push_back(resulting_path);
        }
      }

      if (require_buffer.size() > 0)
        wait(&require_buffer);

      request = parser.parse();
    }

    if (require_buffer.size() > 0)
      wait(&require_buffer);

    lock_guard<mutex> lock(_mutex);
    _being_compiled.erase(path);
    _compiled.insert(path);

    debug(string("Compiled ").append(path));

    _monitor.notify_all();
  } catch (Lexer::Error err) {
    throw Error(Location(path, err.location), err.message);
  } catch (Parser::Error err) {
    throw Error(Location(path, err.token->location), err.reason);
  } catch (Error err) {
    // TODO: Exact position of the require path in current file
    const auto back = Error(Location(path), L"required from this file");
    err.backtrace.push_back(back);
    throw err;
  }
}

void Compiler::wait(vector<fs::path> *required) {
  auto size = required->size();

  for (int i = 0; i < size; i++) {
    auto const path = required->back();

    while (true) {
      unique_lock<mutex> lock(_mutex);

      if (_error || _compiled.count(path))
        return;
      else {
        if (_queued.size() > 0) {
          const auto next_path = _queued.back();
          _queued.pop_back();

          if (!_being_compiled.count(next_path)) {
            _being_compiled.insert(next_path);
            lock.unlock();
            compile_file(next_path);
            break;
          } else {
            _monitor.wait(lock);
          }
        } else {
          _monitor.wait(lock);
        }
      }
    }

    required->pop_back();
  }
}

void Compiler::work() {
  while (true) {
    unique_lock<mutex> lock(_mutex);
    debug("Acquired lock");

    if (_error)
      break;

    if (_queued.size() > 0) {
      const fs::path path = _queued.back();
      _queued.pop_back();

      if (!_compiled.count(path) && !_being_compiled.count(path)) {
        _being_compiled.insert(path);
        lock.unlock();

        try {
          compile_file(path);
          debug("Compiled");
        } catch (Error err) {
          debug("Caught error");
          unique_lock<mutex> lock(_mutex);
          _error = make_unique<Error>(err);
          lock.unlock();
          _monitor.notify_all();
          break;
        }
      }
    } else if (_being_compiled.size() > 0) {
      debug("Waiting for notification...");
      _monitor.wait(lock);
      debug("Received notification");
    } else
      break;
  }

  debug("Done");
}

void Compiler::debug(string message) {
  cerr << "[#" << this_thread::get_id() << "] " << message << "\n";
}
}; // namespace Onyx
