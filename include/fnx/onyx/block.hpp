#include <memory>
#include <variant>

#include "../unit.hpp"
#include "./cst.hpp"
#include "./file.hpp"

namespace FNX {
namespace Onyx {

/// A virtual Onyx code block, typically a result of an Onyx macro or
/// C constant expression evaluation.
class Block : public Unit {
  /// The CST node parenting the block.
  /// The containing unit can be inferred.
  std::shared_ptr<CST::Node> parent_node;
};

} // namespace Onyx
} // namespace FNX
