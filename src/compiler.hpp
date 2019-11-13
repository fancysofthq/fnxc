#pragma once

#include <condition_variable>
#include <experimental/filesystem>
#include <functional>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#include "./sast.hpp"
#include "./shared.hpp"

using namespace std;
namespace fs = experimental::filesystem;

namespace Onyx {
class Compiler {
  vector<fs::path> _queued;
  set<fs::path> _being_compiled;
  set<fs::path> _compiled;

  mutex _mutex;
  condition_variable _monitor;

  static shared_ptr<SAST::Node> _top_level_namespace;
  static mutex _sast_mutex;

public:
  struct Error {
    Location location;
    wstring message;
    vector<Error> backtrace;

    Error(Location location, wstring message);
  };

  Compiler(fs::path entry, u_char workers_count);

private:
  unique_ptr<Error> _error;

  void enqueue(const fs::path path);

  void compile_file(const fs::path path, unsigned short worker_id);

  // Wait until all files at *paths* are compiled.
  // Note that it removes paths from the vector.
  void wait(
      const fs::path from_path,
      vector<fs::path> *paths,
      const unsigned short worker_id);

  void work(int worker_id);

  void debug(ushort worker_id, string message);
};
} // namespace Onyx
