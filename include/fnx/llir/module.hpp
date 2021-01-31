#pragma once

#include <filesystem>
#include <map>
#include <sstream>

#include "../onyx/file.hpp"

namespace FNX {
namespace LLIR {

/// An LLIR module.
struct Module {
  struct Specialization {
    struct Type;

    using Argument = std::pair<
        std::variant<uint8_t, std::string>,
        std::unique_ptr<Type>>;

    struct Type {
      bool is_primitive;
      std::vector<std::string> path;
      std::map<Argument::first_type, Argument::second_type>
          arguments;
    };

    struct Function {
      std::vector<std::string> path;
      std::map<Argument::first_type, Argument::second_type>
          arguments;
      std::unique_ptr<Type> returned_type;
    };
  };

  /// NOTE: First of all, the list of actual (program-defined)
  /// specializations is compiled. Then this list is compared with
  /// the cached specializations list, if any. If they are equal,
  /// then there is no need to re-generate the module.
  ///
  /// NOTE: A specialization is not mapped to the AST in a module.
  ///
  /// OPTIMIZE: It is possible to have some codegen-wide (?) context
  /// to store types referenced multiple times from multiple modules,
  /// e.g. `SBin32`. So that there is no need to store the same
  /// `path` over and over.
  ///

  std::vector<Specialization::Type> type_specializations;
  std::vector<Specialization::Function> function_specializations;

  std::vector<Specialization::Type> cached_type_specializations;
  std::vector<Specialization::Function>
      cached_function_specializations;

  /// The path to the cached module binary representation.
  std::optional<std::filesystem::path> cache_path;

  /// If the module is publicly available (e.g.\ with `$ fnx build
  /// -ellir`), this would be its path. The boolean parameter
  /// specifies whether is the represenation human-readable.
  std::optional<std::tuple<std::filesystem::path, bool>> path;

  /// The binary contents of the module.
  std::stringbuf contents;

  /// The linked Onyx source file.
  std::shared_ptr<Onyx::File> onyx_file;
};

} // namespace LLIR
} // namespace FNX
