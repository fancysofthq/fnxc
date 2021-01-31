#include "../../../include/fnx/onyx/token.hpp"
#include "../../../include/fnx/utils/containers.hpp"

namespace FNX {
namespace Onyx {

VirtualCodeBlock::VirtualCodeBlock(
    unique_ptr<istringstream> source, shared_ptr<Macro> macro) :
    source(move(source)), macro(macro) {}

Placement::Placement(
    Location loc, shared_ptr<VirtualCodeBlock> block) :
    location(loc), virtual_code_block(block) {}

Placement::Placement(Location loc) : location(loc) {}

bool Placement::is_virtual() { return virtual_code_block != NULL; }

TokenBase::TokenBase(Placement placement) : placement(placement) {}

Control::Control(Placement place, Kind kind) :
    TokenBase(place), kind(kind) {}

string Control::debug_source() {
  switch (kind) {
  case Kind::Eof:
    return "<eof>";
  case Kind::Newline:
    return "<newline>";
  case Kind::Space:
    return "<space>";
  default:
    return source();
  }
}

const std::unordered_map<Token::Keyword::Kind, string>
    Token::Keyword::_map = {
        {Let, "let"},
        {Final, "final"},

        {Const, "const"},
        {Mut, "mut"},

        {Do, "do"},
        {End, "end"},

        {Decl, "decl"},
        {Impl, "impl"},
        {Def, "def"},
        {Reimpl, "reimpl"},
        {Undecl, "undecl"},

        {Reopen, "reopen"},
        {Alias, "alias"},

        {As, "as"},
        {To, "to"},

        {Proc, "proc"},
        {Gen, "gen"},
        {Macro, "macro"},
        {Hook, "hook"},
        {Export, "export"},

        {Return, "return"},
        {Break, "break"},
        {Continue, "continue"},

        {Try, "try"},
        {With, "with"},
        {Catch, "catch"},
        {Ensure, "ensure"},
        {Throw, "throw"},

        {If, "if"},
        {Else, "else"},
        {Unless, "unless"},
        {While, "while"},
        {Until, "until"},

        {Namespace, "namespace"},
        {Trait, "trait"},
        {Struct, "struct"},
        {Class, "class"},
        {Enum, "enum"},
        {Flag, "flag"},
        {Annotation, "annotation"},

        {Public, "public"},
        {Protected, "protected"},
        {Private, "private"},

        {Instance, "instance"},
        {Static, "static"},
        {Native, "native"},

        {Compl, "compl"},
        {Incompl, "incompl"},
        {Abstract, "abstract"},

        {ThreadsafeModifier, "threadsafe"},
        {FragileModifier, "fragile"},
        {UnsafeModifier, "unsafe"},

        {ThreadsafeStatement, "threadsafe!"},
        {FragileStatement, "fragile!"},
        {UnsafeStatement, "unsafe!"},

        {Nothrow, "nothrow"},
        {Throws, "throws"},

        {Asm, "asm"},
        {Template, "template"},
        {In, "in"},
        {Out, "out"},

        {True, "true"},
        {False, "false"},

        {And, "and"},
        {Or, "or"},
        {Xor, "xor"},
        {Not, "not"},

        {Ghost, "ghost"},
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
    throw "BUG!";
  else
    return pos->second;
}

string Keyword::source() { return to_string(); }

Comment::Comment(Placement place, string value) :
    TokenBase(place), value(value) {}

Macro::Macro(Placement place, unsigned short delay, string value) :
    TokenBase(place), delay(delay), value(value) {}

ID::ID(Placement place, bool is_unicode, string value) :
    TokenBase(place), is_unicode(is_unicode), value(value) {}

Operator::Operator(Placement place, vector<char> value) :
    TokenBase(place), value(value) {}

Codepoint::Codepoint(
    Placement place, Format fmt, uint32_t val, bool wrapped) :
    TokenBase(place), value(val), wrapped(wrapped), format(fmt) {}

Literal::Literal(
    Placement place, string value, optional<string> suffixes) :
    TokenBase(place), value(value), suffixes(suffixes) {}

string Literal::source() { return value + suffixes.value_or(""); }

CharLiteral::CharLiteral(
    Placement place,
    string literal_value,
    optional<string> literal_suffixes,
    Codepoint cp) :
    Literal(place, literal_value, literal_suffixes), codepoint(cp) {}

StringLiteral::StringLiteral(
    Placement place,
    string literal_value,
    optional<string> literal_suffixes,
    vector<variant<NoBreak, Codepoint>> contents) :
    Literal(place, literal_value, literal_suffixes),
    contents(contents) {}

SymbolLiteral::SymbolLiteral(
    Placement place,
    string literal_value,
    vector<Codepoint> codepoints,
    bool is_unicode) :
    Literal(place, literal_value, nullopt),
    codepoints(codepoints),
    is_unicode(is_unicode) {}

BoolLiteral::BoolLiteral(Placement place, bool value) :
    Literal(place, value ? "true" : "false", nullopt),
    value(value) {}

IntegerlLiteral::IntegerlLiteral(
    Placement place,
    string literal_value,
    optional<string> literal_suffixes,
    optional<bool> sign,
    optional<string> numerical_value,
    Radix radix) :
    Literal(place, literal_value, literal_suffixes),
    sign(sign),
    numerical_value(numerical_value),
    radix(radix) {}

RealLiteral::RealLiteral(
    Placement place,
    string literal_value,
    optional<string> literal_suffixes,
    bool sign,
    Radix radix,
    optional<string> whole,
    optional<string> fractional,
    optional<int> exponent) :
    Literal(place, literal_value, literal_suffixes),
    sign(sign),
    radix(radix),
    whole(whole),
    fractional(fractional),
    exponent(exponent) {}

} // namespace Onyx
} // namespace FNX
