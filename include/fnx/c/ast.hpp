#pragma once

#include <memory>
#include <string>
#include <unordered_map>

using namespace std;

namespace FNX {
namespace C {

// An Abstract Syntax Tree containing C entities.
class AST {
public:
  struct Node;

  // Perform a search for a C entity by its identifier.
  weak_ptr<Node> search(string identifier);

  // Append a node to the AST.
  void append(unique_ptr<Node> node);

private:
  // Nodes of the AST indexed by the identifier.
  unordered_map<string, shared_ptr<Node>> nodes;
};

} // namespace C
} // namespace FNX
