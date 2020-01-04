#pragma once

#include <condition_variable>
#include <experimental/filesystem>
#include <functional>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#include "./macro.hpp"
#include "./sast.hpp"
#include "./shared.hpp"

using namespace std;
namespace fs = experimental::filesystem;

namespace std {
template <> struct hash<fs::path> {
  size_t operator()(fs::path const path) const {
    return std::hash<string>{}(path.generic_u8string());
  }
};
} // namespace std

namespace Onyx {
class Compiler {
  // Variables below control how the source files are being compiled.
  //

  vector<fs::path> _queued;
  set<fs::path> _being_compiled;
  set<fs::path> _compiled;

  mutex _mutex;
  condition_variable _monitor;

  // The top-level SAST node.
  shared_ptr<SAST::Node> _top_level_namespace;
  mutex _sast_mutex;

  // The container to store tokens per file.
  // This includes both tokens from source files and evaluated from macros.
  unordered_map<fs::path, vector<shared_ptr<Token::Base>>> _tokens;

  // Thread-local `Macro` instances.
  unordered_map<thread::id, unique_ptr<Macro>> _macros;

public:
  struct Error {
    Location location;
    wstring message;
    vector<Error> backtrace;

    Error(Location location, wstring message);
  };

  Compiler(fs::path entry, ushort workers_count);

private:
  unique_ptr<Error> _error;

  // Enqueue a file path for compilation.
  void enqueue(const fs::path);

  // Compile the file!
  void compile_file(const fs::path);

  // Wait until all files at *paths* are compiled.
  // Note that it removes paths from the vector.
  void wait(vector<fs::path> *required);

  // Do the job.
  void work();

  void debug(string message);
};
} // namespace Onyx
