#include "fnx/onyx/program.hpp"
#include "fnx/onyx/file.hpp"
#include "fnx/onyx/parser.hpp"
#include <variant>

namespace FNX::Onyx {

bool Program::require(std::vector<std::filesystem::path> paths) {
  // Change of a file or invalidation of its cache triggers
  // invalidation of all its dependants, recursively.
  //

  std::vector<std::future<bool>> futures;

  for (auto &path : paths) {
    futures.push_back(_thread_pool->enqueue([this, path] {
      Onyx::File file(path);
      bool changed;

      if (file.cached_cst_check()) {
        auto deps = file.cached_deps();
        bool any_dep_changed;

        if (deps.has_value()) {
          for (auto &dep : deps.value()) {
            switch (dep.type) {
            case File::CachedDependency::Onyx:
              any_dep_changed |= require(
                  std::get<std::vector<std::filesystem::path>>(
                      dep.paths));
              break;
            case File::CachedDependency::C:
              any_dep_changed |= _import(
                  std::get<std::filesystem::path>(dep.paths));
              break;
            case File::CachedDependency::Lua:
              any_dep_changed |=
                  _load(std::get<std::filesystem::path>(dep.paths));
              break;
            }
          }
        }

        if (any_dep_changed) {
          changed = true;
        } else if (!file.cached_target_check()) {
          changed = true;
        } else if (!file.cached_macro_check()) {
          changed = true;
        } else if (!file.cached_cmacro_check()) {
          changed = true;
        } else if (!file.cached_env_check()) {
          changed = true;
        } else {
          // The file is fully cached. Note that it may have its
          // according module file (LLIR) uncached. Also the list of
          // actual specializations for this file may change.
          //

          file.cached = true;
        }
      } else {
        // The file does not have its CST cached, thus
        // is required to be parsed from scratch.
        //

        changed = true;

        Onyx::Parser parser(file.source_stream());
        auto cst = parser.parse();
        file.set_cst(cst) // TODO: Caches automatically
      }

      // Merge the file CST with global AST.
      _merge(&file.cst);

      // Notify if the file has changed or its cache is invalid.
      return changed;
    }));
  }

  bool any_changed;

  for (auto &future : futures) {
    any_changed |= future.get();
  }

  return any_changed;
}

} // namespace FNX::Onyx
