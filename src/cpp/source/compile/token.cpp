#include "../../header/compile/token.hpp"
#include "../../header/utils/containers.hpp"

namespace Onyx {
namespace Compile {
namespace Token {
const unordered_map<Keyword::Kind, string> Keyword::_map = {
    {Var, "var"},
    {Const, "const"},

    {Do, "do"},
    {End, "end"},

    {Def, "def"},
    {Macro, "macro"},

    {Return, "return"},
    {Convey, "convey"},
    {Yield, "yield"},
    {Break, "break"},
    {Continue, "continue"},

    {Begin, "begin"},
    {Catch, "catch"},
    {Rescue, "rescue"},
    {Raise, "raise"},

    {If, "if"},
    {Unless, "unless"},
    {While, "while"},
    {Until, "until"},

    {Namespace, "namespace"},
    {Module, "module"},
    {Primitive, "primitive"},
    {Struct, "struct"},
    {Class, "class"},
    {Enum, "enum"},
    {Flag, "flag"},
    {Annotation, "annotation"},

    {Protected, "protected"},
    {Private, "private"},

    {Static, "static"},
    {Threadlocal, "threadlocal"},

    {Threadsafe, "threadsafe"},
    {Volatile, "volatile"},
    {Unsafe, "unsafe"},
    {Unordered, "unordered"},
};

const set<string> Value::_comment_intrinsics = {
    "no-cache",
    "ditto",
    "super",
    "doc",
    "nodoc",
};

const unordered_map<string, Keyword::Kind> Keyword::_invmap =
    inverse_map(_map);

optional<Keyword::Kind> Keyword::from_string(string cmp) {
  auto pos = _invmap.find(cmp);

  if (pos == _invmap.end())
    return nullopt;
  else
    return pos->second;
}

string Keyword::to_string() {
  auto pos = _map.find(kind);

  if (pos == _map.end())
    throw "BUG";
  else
    return pos->second;
}

bool Value::is_comment_intrinsic(string checked) {
  return _comment_intrinsics.count(checked) > 0;
}
} // namespace Token
} // namespace Compile
} // namespace Onyx
