#include "./sast.hpp"
#include <iterator>

namespace Onyx {
namespace SAST {
void Declaration::dump(wostream *buffer, ushort tabs) {
  *buffer << SAST::tabs(tabs) << L"DECLARATION";
}

void Namespace::dump(wostream *buffer, ushort tabs) {
  bool top_level = parent_namespace.get();

  if (top_level)
    *buffer << SAST::tabs(tabs) << L"namespace " << name << "\n";

  auto function = functions.begin();

  while (function != functions.end()) {
    (*function)->dump(buffer, tabs + (top_level && 1 || 0));
    function++;
  }

  if (top_level)
    *buffer << SAST::tabs(tabs) << L"end\n";
}

void FunctionDefinition::dump(wostream *buffer, ushort tabs) {
  *buffer << SAST::tabs(tabs) << L"def ";
  prototype->dump(buffer, tabs);
  *buffer << SAST::tabs(tabs) << L"end\n";
}

void Body::dump(wostream *buffer, ushort tabs) {}

void FunctionPrototype::dump(wostream *buffer, ushort tabs) {
  if (holds_alternative<shared_ptr<Token::ID>>(name))
    *buffer << get<shared_ptr<Token::ID>>(name)->value;
  else
    *buffer << get<shared_ptr<Token::Op>>(name)->value;

  if (args.size() > 0) {
    *buffer << "(";
    auto arg = args.begin();

    while (arg != args.end()) {
      (*arg)->dump(buffer, tabs + 1);
      arg++;

      if (arg != args.end())
        *buffer << ", ";
    }

    *buffer << ")";
  }

  *buffer << "\n";
}

void FunctionArgumentDeclaration::dump(wostream *buffer, ushort tabs) {
  *buffer << name->value;
}
} // namespace SAST
} // namespace Onyx
