#include <experimental/filesystem>
#include <iostream>
#include <locale>
#include <thread>

using namespace std;
namespace fs = experimental::filesystem;

#include "./compiler.hpp"
#include "./utils/log.hpp"

struct StandardError : std::exception {
  StandardError(const string message) :
      std::exception(message.c_str()) {}
};

// Commands:
//
//   * onyxc               — run the REPL
//   * onyxc [file]        — run [file] in JIT mode
//   * onyxc run [file]    — ditto
//   * onyxc build [file]  — build [file] in AOT mode
//   * onyxc api [file]    — generate API for [file]
//   * onyxc format [file] — format an Onyx source file
//
int main(int argc, char *argv[]) {
  locale::global(locale("en_US.UTF-8"));
  const string progname = string(argv[0]);

  try {
    if (argc == 1)
      throw StandardError("REPL is not implemented yet");

    string arg = string(argv[1]);

    // The `build` command builds an Onyx program in AOT mode.
    //
    // ```sh
    // > onyxc build main.nx -o ./bin/main
    // ```
    if (arg == "build") {
      if (argc < 3)
        throw StandardError("Expected filename");

      string filename = argv[2];

      if (filename.empty())
        throw StandardError("Expected filename");

      bool is_jobs;
      int jobs = thread::hardware_concurrency();

      for (int i = 3; i < argc; i++) {
        if (is_jobs) {
          jobs = atoi(argv[i]);

          if (!(jobs > 0))
            throw StandardError("Expected jobs count to be > 0");

          is_jobs = false;
        }

        // TODO: Allow `-jN`
        if (!string(argv[i]).compare("-j"))
          is_jobs = true;
      }

      if (is_jobs)
        throw StandardError("Expected explicit "
                            "number of jobs after -j");

      trace("Compiling program " + filename + "...");
      Onyx::Compiler::compile(
          fs::path(filename), (unsigned char)jobs);
      trace("Compiled program");
    } else
      throw StandardError("Unknown command " + arg);
  } catch (StandardError &e) {
    cerr << "Error: " << e.what();
    exit(EXIT_FAILURE);
  } catch (Onyx::Compiler::Panic &p) {
    if (!p.location.path.empty()) {
      cerr << "Panic at " << p.location.path << ":"
           << p.location.begin_row << ":" << p.location.begin_column
           << ": " << p.message << "\n";
    } else {
      cerr << "Panic: " << p.message << "\n";
    }

    exit(EXIT_FAILURE);
  }

  return 0;
}
