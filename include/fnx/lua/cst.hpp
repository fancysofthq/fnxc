#pragma once

#include <memory>
#include <vector>

namespace FNX {
namespace Lua {

class CST {
  struct Node {
    std::shared_ptr<Node> parent;
  };

  struct Root : Node {
    std::vector<std::shared_ptr<Node>> children;
  };
};

} // namespace Lua
} // namespace FNX
