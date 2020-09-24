#pragma once

#include <istream>
#include <memory>
#include <stdint.h>
#include <utility>
#include <variant>

#include "./file.hpp"

namespace FNX {
namespace Onyx {

/**
 * @brief A virtual macro emitting Onyx source code.
 *
 * @details
 * Its `source_stream` shall be written into by `Parser` prior to and
 * remain intact after evaluation for future reference.
 *
 * A `Macro` instance contains its own macro execution engine
 * instance, e.g. a Lua state. That said, multiple `Macro`
 * instances may evaluate their macros simultaneously. Precautions
 * shall be made during access to the shared state (e.g. Onyx AST).
 *
 * @note A macro does not have its own AST. Instead, it emits raw
 * source code to be consumed separately.
 */
class Macro : public Unit {
public:
  /**
   * @brief Precompiled source code used in delayed macros.
   */
  std::iostream precompiled_source_stream;

  /**
   * @brief The emitted Onyx code stream.
   *
   * It may be written by calling `nx.emit()` function from
   * macro code. Then the stream is read as Onyx source code. Note
   * that the stream contents shall be preserved for future
   * reference.
   */
  std::iostream emit_stream;

  /**
   * @brief The parent unit of the macro.
   *
   * It may be an `Onyx::File` or another `Onyx::Macro`.
   */
  const std::variant<std::shared_ptr<File>, std::shared_ptr<Macro>>
      parent;

  /**
   * @brief Indices of parent unit tokens comprising the macro.
   *
   * A single index means it is a simple macro. For example:
   *
   * @code{.nx}
   * {% do_someting() %}
   * @endcode
   *
   * A pair of indices means it is a complex macro. For example,
   *
   * @code{.nx}
   * {% for i = 0, 2 do %}
   *   Std.puts("i = {{ i }}")
   * {% end %}
   * @endcode
   *
   * is a complex macro spanning over multiple lines consisting of a
   * lot of tokens, so it would be a pair of indices.
   */
  std::variant<uint32_t, std::pair<uint32_t, uint32_t>>
      parent_token_idx;

  /**
   * @brief Check if the macro is complex.
   *
   * This is determined by the option of `parent_token_idx`.
   */
  bool is_complex();

  Macro(
      std::variant<std::shared_ptr<File>, std::shared_ptr<Macro>>
          parent,
      uint32_t begin_token_idx);

private:
  /// Some sort of macro engine state, e.g. a Lua state instance.
  void *_state;
};

} // namespace Onyx
} // namespace FNX
