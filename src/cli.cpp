#include <filesystem>
#include <iostream>
#include <locale>
#include <regex>
#include <sqlite3.h>
#include <string>
#include <thread>

using namespace std;
namespace fs = filesystem;

// #include "./cpp/header/app/aot.hpp"
#include "./cpp/header/utils/log.hpp"

struct StandardError : std::exception {
  StandardError(const string message) :
      std::exception(message.c_str()) {}
};

Verbosity verbosity = Trace;

// Commands:
//
//   // * nxc               — run the REPL
//   // * nxc repl          — ditto
//   // * nxc [file]        — run [file] in JIT mode
//   // * nxc run [file]    — ditto
//   * nxc build [file]  — build [file] in AOT mode
//   // * nxc api [file]    — generate API for [file]
//   // * nxc format [file] — format an Onyx source file
//
// TODO: Rename `api` to `doc`?
int main(int argc, char *argv[]) {
  locale::global(locale("en_US.UTF-8"));

  // The first argument is the program name
  const string progname = string(argv[0]);

  try {
    if (argc == 1)
      // `$ nxc` implies `$ nxc repl`
      throw StandardError("REPL is not implemented yet");

    string arg = string(argv[1]);

    // The `build` command builds an Onyx program in AOT mode.
    //
    // ```sh
    // $ onyxc build -imain.nx -o./bin/main -j3
    // ```
    if (arg == "build") {
      fs::path input_path;
      fs::path output_path;

      // The number of threads to utilize.
      // Platform maximum by default.
      unsigned short jobs_count = thread::hardware_concurrency();

      for (int i = 2; i < argc; i++) {
        arg = string(argv[i]);
        trace(arg);

        smatch sm;

        if (regex_match(arg, sm, regex("^-([if])(.*)"))) {
          fs::path path(sm[2].str());

          if (path.empty())
            throw StandardError(
                "Expected file name after " + sm[1].str());

          if (sm[1].compare("-i")) {
            if (!input_path.empty())
              throw StandardError("Input path already set");

            input_path = path;

            trace(
                "Set `input_path` to \"" + input_path.string() +
                "\"");

            if (input_path.empty())
              throw StandardError("Input path shall not be empty");

            if (!fs::exists(input_path))
              throw StandardError(
                  "Input file " + input_path.string() +
                  " does not exist");
          } else {
            if (!output_path.empty())
              throw StandardError("Output path already set");

            output_path = path;

            trace(
                "Set `output_path` to \"" + output_path.string() +
                "\"");

            if (output_path.empty())
              throw StandardError("Output path shall not be empty");
          }
        } else if (regex_match(arg, sm, regex("^-j(\\d+)"))) {
          jobs_count = std::stoi(sm[1]);

          if (!(jobs_count > 0))
            throw StandardError("Expected jobs count to be > 0");
        }
      }

      if (input_path.empty())
        throw StandardError("Expected input path to be set (-i)");

      if (output_path.empty()) {
        output_path = input_path.parent_path() / input_path.stem();

        debug(
            "Output path automatically set to \"" +
            output_path.string() + "\"");
      }

      ldebug() << "Jobs count set to " << jobs_count;

      // auto aot =
      //     Onyx::App::AOT(input_path, output_path, false,
      //     jobs_count);

      // debug("Building " + input_path.string() + "...");
      // aot.compile();
      // debug("Successfully built the program");
    } else
      throw StandardError("Unknown command " + arg);
  } catch (StandardError &e) {
    cerr << "Error: " << e.what();
    exit(EXIT_FAILURE);
    // } catch (Onyx::Compiler::Panic &p) {
    //   cerr << "Panic! " << p.what() << "\n";

    //   while (!p.backtrace.empty()) {
    //     auto loc = p.backtrace.top();

    //     cerr << "\t";
    //     cerr << loc.unit.get()->path;
    //     cerr << ":" << loc.begin.row << ":" << loc.begin.col;
    //     cerr << "\n";

    //     p.backtrace.pop();
    //   }

    //   exit(EXIT_FAILURE);
  } catch (...) {
    cout << "[BUG] Unhandled exception!\n";
    exit(EXIT_FAILURE);
  }

  return 0;
}
