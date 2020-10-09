#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>

#include "../c/cst.hpp"
#include "../cnx/cst.hpp"
#include "../lua/block.hpp"
#include "./token.hpp"

namespace FNX {
namespace Onyx {

/// A Concrete Syntax Tree unique per compilation unit.
///
/// A CST parser does not perform any code analysis, i.e. does
/// neither lookup identifers nor trigger specializations. It does
/// not evaluate neither Onyx macros nor C constant expressions.
///
/// A CST is merely a binary representation of the source code. It is
/// even possible to recreate the source code from a CST. Some
/// details would be lost, though, such as exact space sequences.
class CST {
public:
  struct Node;
  struct ComptimeStatement;
  struct ArgumentPass;
  struct Path;
  struct Alias;
  struct ID;
  struct Symbol;
  struct Label;
  struct Operator;

  /// A reference statement. It may be either `let`, `final`, `get`
  /// or `set` declaration with optional alias, name, type
  /// restriction and value with optional documentation, annotations
  /// and modifiers.
  struct ReferenceStatement;

  struct SubroutineStatement;
  struct TypeStatement;
  struct ComplexTypeReference;
  struct TypeExpression;
  struct UnaryTypeOperation;
  struct BinaryTypeOperation;
  struct EntityReference;

  struct NumericLiteral;
  struct StringLiteral;
  struct CharLiteral;
  struct ContainerLiteral;
  struct QuotedStringLiteral;

  /// A runtime expression.
  struct Expression;

  struct Bracket;

  struct MatchAll;

  struct Macro;
  struct CConstExpr;
  struct ExportStatement;
  struct RequireDirective;
  struct ImportDirective;

  /// A pair of brackets.
  using Brackets = std::pair<Bracket, Bracket>;

  /// A reference to a single type with arguments.
  using RealTypeReference = EntityReference;

  /// The unit containing this CST.
  std::variant<std::shared_ptr<File>, std::shared_ptr<Block>> unit;

}; // namespace CST
} // namespace Onyx
} // namespace FNX

namespace FNX::Onyx {

/// A CST node.
struct CST::Node {
  std::shared_ptr<Node> parent;
};

/// An identifier node, e.g.\ `foo` or `` `foo` ``.
struct CST::ID : Node {
  std::shared_ptr<Token::ID> token;

  /// The normalized identifier name without backticks.
  /// @note Explicit identifiers are not normalized.
  std::u32string name;
};

/// A symbol node, e.g.\ `:foo` or `` :`foo` ``.
struct CST::Symbol : Node {
  std::shared_ptr<Token::ID> token;

  /// The normalized symbol name without backticks.
  /// @note Explicit symbols are not normalized.
  std::u32string name;
};

/// A label node, e.g.\ `foo:` or `` `foo`: ``.
struct CST::Label : Node {
  std::shared_ptr<Token::ID> token;

  /// The normalized label name without backticks.
  /// @note Explicit labels are not normalized.
  std::u32string name;
};

/// An operator node.
struct CST::Operator : Node {
  std::shared_ptr<Token::Operator> token;

  /// The normalized Unicode operator value.
  std::u32string value;
};

/// A `*` token may be used as a "match all" argument in aliases.
struct CST::MatchAll : Node {
  std::shared_ptr<Token::Operator> token;
};

/// A bracket node.
struct CST::Bracket : Node {
  enum Kind {
    Paren,  ///< @c ( or @c ) .
    Curly,  ///< @c { or @c } .
    Square, ///< @c [ or @c ] .
    Angle,  ///< @c < or @c > .
    Pipe,   ///< @c | .
  };

  const Kind kind;
  const bool is_closing;
  std::shared_ptr<Token::Punctuation> token;
};

/// An Onyx macro node.
struct CST::Macro : Node {
  enum Kind {
    Immediate,
    Delayed,
    Final,
  };

  const std::shared_ptr<Token::Punctuation> opening_token;
  const std::shared_ptr<Token::Punctuation> closing_token;

  const Kind kind;
  const bool is_emitting;

  /// The containing Lua block.
  const std::shared_ptr<Lua::Block> lua_block;

  Macro(
      Kind,
      bool is_emitting,
      std::shared_ptr<Lua::Block> lua_block,
      std::shared_ptr<Token::Punctuation> opening_token,
      std::shared_ptr<Token::Punctuation> closing_token);
};

/// A C constant expression node.
struct CST::CConstExpr : Node {
  /// The `$` token.
  const std::shared_ptr<Token::Punctuation> c_token;

  /// The curly brackets pair.
  const Brackets brackets;

  /// The C CST root node.
  const std::shared_ptr<C::CST> root;
};

/// An `export` statement.
struct CST::ExportStatement : Node {
  /// The `export` keyword token.
  std::shared_ptr<Token::Keyword> keyword;

  /// Whether is it a block export statement.
  bool is_block;

  /// The CNX CST root node.
  std::shared_ptr<CNX::CST::Node> root;
};

/// A compile-time statement, such a subroutine or type statement, or
/// an `alias` statement.
struct CST::ComptimeStatement : Node {};

/// A path to a entity excluding the terminator (i.e.\ the latest
/// identifier) arguments. For example, e.g.\x20 `Foo<T>::Bar`, but
/// not `Foo::Bar<T>`.
struct CST::Path : Node {
  // IDEA: `Strukt:+::bar` for entities defined within an instance
  // subroutine. Seems legit?
  //

  /// A part delimeter.
  enum class Delimeter {
    Instance, // `:foo`
    Static,   // `::foo`
  };

  /// Parts of the path.
  /// @note Delimeter shall not be empty for successive parts.
  const std::vector<std::pair<
      std::optional<Delimeter>, // NOTE: Here
      std::variant<
          std::shared_ptr<EntityReference>,
          std::shared_ptr<Operator>>>>
      parts;
};

/// Argument passing, e.g.\ `foo: bar`,
/// `[0]: bar`, `bar` or even `*`.
struct CST::ArgumentPass : Node {
  const std::optional<std::variant<
      std::shared_ptr<Label>,
      std::shared_ptr<NumericLiteral>>>
      name;

  const std::
      variant<std::shared_ptr<Expression>, std::shared_ptr<MatchAll>>
          value;
};

/// A reference to a single entity with path and arguments.
/// It may be a subroutine or a type reference.
/// For example, `Foo<T>::Bar<U>` or `baz(42, x: 17)`.
struct CST::EntityReference : Node {
  /// Optional brackets wrapping the entity itself. For example,
  /// `<T,>` or `(T,)`. Comma is required for vectors and tuples.
  const std::optional<Brackets> brackets;

  /// Path to the entity.
  const Path path;

  /// The reference arguments list. Empty brackets are common.
  /// Subroutine calls shall use parentheses even with zero arity.
  const std::optional<std::tuple<
      Brackets,
      std::optional<std::vector<std::shared_ptr<ArgumentPass>>>>>
      arguments;
};

/// An alias statement, e.g.\ `alias foo(a: T, *) to bar(a, *)`.
struct CST::Alias : ComptimeStatement {
  // TODO: Document that `alias foo(a, *), bar(b: T, *) to qux(*)`
  // would panic, because there must be the same set of arguments.

  // Points to the `alias` keyword.
  const Token::Keyword statement_keyword;

  const std::variant<
      std::shared_ptr<SubroutineStatement>,
      std::shared_ptr<TypeStatement>>
      parent;

  const std::vector<EntityReference> sources;
  const EntityReference target;

  // An `alias` statement may contain multiple `forall` clauses.
  //
  // ```nx
  // alias foo(x: T) = bar(x) forall T~Numeric
  // alias foo(x: ~Numeric) = bar(x) # Ditto
  // ```
  const std::optional<std::vector<
      std::tuple<Token::Keyword, std::vector<ComplexTypeReference>>>>
      foralls;
};

/// A type statement, which may be either a `primitive`, `namespace`,
/// `trait`, `struct`CST::, `class`, `enum`, `flag` or `distinct
/// alias` declaration (`decl`), implementation (`impl`), definition
/// (`def`) or reopening (`reopen`) with optional documentation,
/// annotations and modifiers.
struct CST::TypeStatement : ComptimeStatement {
  // A type statement kind.
  enum Kind {
    Primitive,
    Namespace,
    Trait,
    Struct,
    Class,
    Enum,
    Flag,
    DistinctAlias,
    Reopening,
  };

  // A type may be declared within another type or
  // subroutine declaration or specialization.
  // Would be empty for the top-level namespace.
  const std::optional<std::variant<
      std::shared_ptr<SubroutineStatement>,
      std::shared_ptr<TypeStatement>>>
      parent;

  const std::optional<std::vector<RealTypeReference>> annotations;
  const std::optional<std::vector<Token::Comment>> documentation;
  const std::optional<std::vector<Token::Keyword>> modifiers;

  // Points to the declaration statement keyword, e.g. `namespace`
  // or `distinct`. Would be empty for the top-level namespace.
  // Points to `distrinct` iff is a distinct alias.
  const std::optional<Token::Keyword> statement_keyword;

  // Would be empty for the top-level namespace.
  const std::optional<Path> path;

  // A statement generic argument definitions.
  const std::optional<std::vector<ReferenceStatement>> arguments;

  // Replaces `path` and `arguments`.
  // Only set iff is a distinct alias.
  const std::optional<Alias> alias;

  // Replaces `path` and `arguments`.
  // Only set iff is a reopening.
  const std::optional<RealTypeReference> reference;

  // May be set for reopenings and distinct aliases.
  const std::optional<std::vector<
      std::pair<Token::Keyword, std::vector<ComplexTypeReference>>>>
      foralls;

  // An std::optional `where` delayed (non-final) macro.
  const std::optional<std::shared_ptr<Macro>> where;
};

/// A subroutine statement, which may be either a procedure (`proc`)
/// or generator (`gen`) declaration (`decl`), implementation
/// (`impl`), definition (`def`), moving (`moveimpl`),
/// reimplementation (`reimpl`) or undeclaration (`undecl`) with
/// optional documentation, annotations and modifiers.
struct CST::SubroutineStatement : ComptimeStatement {
  enum StatementKind {
    Declaration,        // `decl foo()`
    Implementation,     // `impl foo();`
    Definition,         // `def foo();`
    MoveImplementation, // `moveimpl foo() to bar`
    ReImplementation,   // `reimpl foo();`, `reimpl foo() as bar;`
    UnDeclaration,      // `undecl foo()`
  };

  enum SubroutineKind {
    Procedure, // `decl proc foo()`, default
    Generator, // `decl gen foo()`, inferred if args contain a block
  };

  // A subroutine statement may even be contained
  // within a subroutine statement. In other words,
  // a function may be declared within a function.
  const std::variant<
      std::shared_ptr<SubroutineStatement>,
      std::shared_ptr<TypeStatement>>
      parent;

  const std::optional<std::vector<RealTypeReference>> annotations;
  const std::optional<std::vector<Token::Comment>> documentation;
  const std::optional<std::vector<Token::Keyword>> modifiers;

  // See `StatementKind`.
  const Token::Keyword statement_keyword;

  // May be absent. See `SubroutineKind`.
  const std::optional<Token::Keyword> kind_keyword;

  // The subroutine identifier.
  const Path id;

  // For (un)declarations and movings, it would contain reference
  // declarations, e.g. `(foo: T)`, in other cases these are
  // reference definitions, e.g. `(foo: bar : T)`.
  //
  // Note that it would be a panic to have more than needed in
  // for declaration, e.g. `decl foo(let arg: x : T)`.
  const std::optional<std::vector<ReferenceStatement>> arguments;

  // Only applicable to movings (e.g. `moveimpl foo() [to bar]`)
  // and re-implementations (e.g. `reimpl foo() [as bar];`).
  const std::optional<std::pair<Token::Keyword, Token::ID>> new_id;

  // There may be multiple foralls,
  // e.g. `forall T<U> ~ V, W forall U`.
  const std::optional<std::vector<
      std::pair<Token::Keyword, std::vector<ComplexTypeReference>>>>
      foralls;

  // An std::optional `where` delayed (non-final) macro.
  const std::optional<std::shared_ptr<Macro>> where;

  // Only set for (re)implementations, excluding native ones.
  const std::optional<std::vector<Expression>> body;
};

/// An unary logical type operation.
/// Currently only `!` (NOT) is supported.
struct CST::UnaryTypeOperation : Node {
  enum Operator {
    Not, // E.g. `!T`
  };

  // NOTE: `operator` is a keyword in C++.
  const Token::Operator op;

  const std::unique_ptr<TypeExpression> operand;

  Operator get_op() const;
};

/// An binary logical type operation. `&&` (AND), `||` (OR) and `^`
/// (XOR) are currently supported.
struct CST::BinaryTypeOperation : Node {
  enum Operator {
    And, // E.g. `A && B`
    Or,  // E.g. `A || B`
    Xor, // E.g. `A ^ B`
  };

  // NOTE: `operator` is a keyword in C++.
  const Token::Operator op;

  const std::unique_ptr<TypeExpression> left_operand;
  const std::unique_ptr<TypeExpression> right_operand;

  Operator get_op() const;
};

/// A logical type expression comprising unary and binary logical
/// operations, nesting with parentheses and real types (i.e. no
/// complex types).
struct CST::TypeExpression : Node {
  const std::optional<Brackets> brackets;

  const std::variant<
      std::unique_ptr<UnaryTypeOperation>,
      std::unique_ptr<BinaryTypeOperation>,
      std::shared_ptr<RealTypeReference>>
      expr;
};

/// A complex type reference, i.e.\ consisting of real and imaginary
/// parts. If either is missing, it may be inferred in some cases.
struct CST::ComplexTypeReference : Node {
  std::optional<RealTypeReference> real;
  std::optional<TypeExpression> imaginary;
};

} // namespace FNX::Onyx
