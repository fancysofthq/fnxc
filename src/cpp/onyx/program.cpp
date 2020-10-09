#include "fnx/onyx/program.hpp"
#include "fnx/onyx/file.hpp"
#include "fnx/onyx/parser.hpp"

namespace FNX::Onyx {

bool Program::require(std::vector<std::filesystem::path> paths) {
  std::vector<std::future<bool>> futures;
  bool changed;

  for (auto &path : paths) {
    futures.push_back(_thread_pool->enqueue([this, path, &changed] {
      Onyx::File file(path);

      if (file.cache_is_physically_invalid()) {
        // Wait for the file to be reparsed
        _enqueue(file).get();
        return;
      }

      auto deps = file.cache_dependencies();

      if (deps.has_value()) {
        for (auto &dep : deps.value()) {
          switch (dep.type) {
          case File::Dependency::Onyx:
            require(dep.paths);
            break;
          case File::Dependency::C:
            _import(dep.paths);
            break;
          case File::Dependency::Lua:
            _load(dep.paths);
            break;
          }
        }
      }

      if (!file.has_changed() && !file.cached_macro_call_differs()) {
        for (auto path : *&(file.onyx_dependendees)) {
          bool has_changed = require({path});

          if (has_changed)
            changed = true;
        }
      }

      auto cst = file.cached_cst();

      if (!cst.has_value()) {
        Onyx::Parser parser(file.source_stream());
        cst = parser.parse();
        file.cache(&cst.value());
      }

      _merge(&cst.value());
    }));
  }

  for (auto &future : futures) {
    future.get();
  }
}

} // namespace FNX::Onyx
