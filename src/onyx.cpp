#include <experimental/filesystem>
#include <iostream>
#include <locale>
#include <thread>

using namespace std;
namespace fs = experimental::filesystem;

#include "./compiler.hpp"

// Commands:
//
//   * onyx              — run REPL
//   * onyx [file]       — run [file] in JIT mode
//   * onyx build [file] — build [file] in AOT mode
//   * onyx api [file]   — generate API for [file]
//
int main(int argc, char *argv[]) {
  const string progname = string(argv[0]);
  locale::global(locale("en_US.UTF-8"));

  try {
    if (argc == 1)
      throw "REPL is not implemented yet";

    string arg = string(argv[1]);

    // The `build` command builds an Onyx program in AOT mode.
    //
    // ```sh
    // > onyx build main.nx -o ./bin/main
    // ```
    if (arg == "build") {
      if (argc < 3)
        throw "Expected filename";

      string filename = argv[2];

      if (filename.empty())
        throw "Expected filename";

      bool is_jobs;
      int jobs = thread::hardware_concurrency();

      for (int i = 3; i < argc; i++) {
        if (is_jobs) {
          jobs = stoi(argv[i]);

          if (!(jobs > 0))
            throw "Expected jobs count to be > 0";

          is_jobs = false;
        }

        if (!string(argv[i]).compare("-j"))
          is_jobs = true;
      }

      if (is_jobs)
        throw "Expected explicit number of jobs after -j";

      try {
        cerr << "[D] Compiling...\n";
        auto compiler = Onyx::Compiler(fs::path(filename), jobs);
        cerr << "[D] Compiled\n";
      } catch (Onyx::Compiler::Error err) {
        if (!err.location.path.empty()) {
          wcerr << "Panic at " << err.location.path << ":"
                << err.location.begin_row << ":" << err.location.begin_column
                << ": " << err.message << "\n";
        } else {
          wcerr << "Panic: " << err.message << "\n";
        }

        exit(EXIT_FAILURE);
      }
    } else
      throw "Unknown command " + arg;
  } catch (const string e) {
    cout << "Error: " << e << "\n";
    exit(EXIT_FAILURE);
  } catch (const char *e) {
    cout << "Error: " << e << "\n";
    exit(EXIT_FAILURE);
  }

  return 0;
}
