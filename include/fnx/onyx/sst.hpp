#pragma once

#include <optional>

#include "./token.hpp"

namespace Onyx {
namespace Compiler {
// A Source Syntax Tree unique per compilation unit.
//
// It is built from source code with immediately evaluated macros.
// An SST does not perform any code compilation, i.e. does neither
// lookup identifers nor trigger specializations.
// An SST is merely a binary representation of the source code
// after immediate macros compilation.
//
// NOTE: SST does not preserve source file structure.
//
// TODO: SST includes immediate macros,
// but they're ignored upon parsing the AST.
// No, it's not needed, as the token is saved on the stack.
namespace SST {

// An SST node.
struct Node;

// A compile-time statement, such a subroutine or type statement, or
// an alias statement.
struct ComptimeStatement;

// Argument passing, for example `foo: bar`,
// `[0]: bar`, `bar` or even `*`.
struct ArgumentPass;

// Some path to a entity excluding terminator
// (i.e. the latest identifier) arguments, e.g.
// `Foo<T>::Bar`, but not `Foo::Bar<T>`.
struct Path;

// An alias statement, e.g. `alias foo(a: T, *) to qux(a, *)`.
struct Alias;

// A reference statement, which may be either `let`, `final`, `get`
// or `set` declaration with optional alias, name, type restriction
// and value with optional documentation, annotations and modifiers.
struct ReferenceStatement;

// A subroutine statement, which may be either a procedure (`proc`)
// or generator (`gen`) declaration (`decl`), implementation
// (`impl`), definition (`def`), moving (`moveimpl`),
// reimplementation (`reimpl`) or undeclaration (`undecl`) with
// optional documentation, annotations and modifiers.
struct SubroutineStatement;

// A type statement, which may be either a `primitive`, `namespace`,
// `trait`, `struct`, `class`, `enum`, `flag` or `distinct alias`
// declaration (`decl`), implementation (`impl`), definition (`def`)
// or reopening (`reopen`) with optional documentation, annotations
// and modifiers.
struct TypeStatement;

// A complex type reference, i.e. consisting of real and imaginary
// parts. If either is missing, it may be inferred in some cases.
struct ComplexTypeReference;

// A logical type expression comprising uanry and binary logical
// operations, nesting with parentheses and real types (i.e. no
// complex types).
struct TypeExpression;

// An unary logical type operation. Currently only `!` (NOT) is
// supported.
struct UnaryTypeOperation;

// An binary logical type operation. `&&` (AND), `||` (OR) and `^`
// (XOR) are currently supported.
struct BinaryTypeOperation;

// A reference to a single entity with path and arguments.
// It may be a subroutine or a type reference.
// For example, `Foo<T>::Bar<U>` or `baz(42, x: 17)`.
struct EntityReference;

// A runtime expression.
struct Expression;

// A `*` token may be used as a "match all" argument in aliases.
using MatchAll = Token::Control;

// A pair of brackets: `[]`, `()`, `<>` or `{}`.
using Brackets = pair<Token::Control, Token::Control>;

// A reference to a single type with arguments.
using RealTypeReference = EntityReference;

} // namespace SST
} // namespace Compiler
} // namespace Onyx

namespace Onyx {
namespace Compiler {
namespace SST {

struct Node {};

struct ComptimeStatement : Node {};

struct Path : Node {
  // A part delimeter.
  //
  // NOTE: `Strukt:+::bar` for entities
  // defined within an instance subroutine.
  // Seems legit?
  //
  enum Delimeter {
    Instance, // `:foo`
    Static,   // `::foo`
  };

  // Delimeter shall not be empty for successive parts.
  const vector<pair<
      optional<Delimeter>,
      variant<Token::ID, Token::Macro, Token::Operator>>>
      parts;
};

struct ArgumentPass : Node {
  const optional<variant<Token::ID, Token::IntegerlLiteral>> name;
  const variant<unique_ptr<Expression>, MatchAll> value;
};

struct EntityReference : Node {
  // Optional brackets wrapping the entity itself. For example, `<T>`
  // or `(T)`. Note that a comma is required for a vector or tuple.
  const optional<Brackets> brackets;

  // Path to the entity.
  const Path path;

  // The reference arguments. Subroutine calls shall
  // use parentheses even with zero arity.
  const optional<tuple<Brackets, optional<vector<ArgumentPass>>>>
      arguments;
};

struct Alias : ComptimeStatement {
  // TODO: Document that `alias foo(a, *), bar(b: T, *) to qux(*)`
  // would panic, because there must be the same set of arguments.

  // Points to the `alias` keyword.
  const Token::Keyword statement_keyword;

  const variant<
      shared_ptr<SubroutineStatement>,
      shared_ptr<TypeStatement>>
      parent;

  const vector<EntityReference> sources;
  const EntityReference target;

  // An `alias` statement may contain multiple `forall` clauses.
  //
  // ```nx
  // alias foo(x: T) = bar(x) forall T~Numeric
  // alias foo(x: ~Numeric) = bar(x) # Ditto
  // ```
  const optional<
      vector<tuple<Token::Keyword, vector<ComplexTypeReference>>>>
      foralls;
};

struct TypeStatement : ComptimeStatement {
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
  const optional<variant<
      shared_ptr<SubroutineStatement>,
      shared_ptr<TypeStatement>>>
      parent;

  const optional<vector<RealTypeReference>> annotations;
  const optional<vector<Token::Comment>> documentation;
  const optional<vector<Token::Keyword>> modifiers;

  // Points to the declaration statement keyword, e.g. `namespace`
  // or `distinct`. Would be empty for the top-level namespace.
  // Points to `distrinct` iff is a distinct alias.
  const optional<Token::Keyword> statement_keyword;

  // Would be empty for the top-level namespace.
  const optional<Path> path;

  // A statement generic argument definitions.
  const optional<vector<ReferenceStatement>> arguments;

  // Replaces `path` and `arguments`.
  // Only set iff is a distinct alias.
  const optional<Alias> alias;

  // Replaces `path` and `arguments`.
  // Only set iff is a reopening.
  const optional<RealTypeReference> reference;

  // May be set for reopenings and distinct aliases.
  const optional<
      vector<pair<Token::Keyword, vector<ComplexTypeReference>>>>
      foralls;

  // An optional `where` delayed (non-final) macro.
  const optional<Token::Macro> where;
};

struct SubroutineStatement : ComptimeStatement {
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
  const variant<
      shared_ptr<SubroutineStatement>,
      shared_ptr<TypeStatement>>
      parent;

  const optional<vector<RealTypeReference>> annotations;
  const optional<vector<Token::Comment>> documentation;
  const optional<vector<Token::Keyword>> modifiers;

  // See `StatementKind`.
  const Token::Keyword statement_keyword;

  // May be absent. See `SubroutineKind`.
  const optional<Token::Keyword> kind_keyword;

  // The subroutine identifier.
  const Path id;

  // For (un)declarations and movings, it would contain reference
  // declarations, e.g. `(foo: T)`, in other cases these are
  // reference definitions, e.g. `(foo: bar : T)`.
  //
  // Note that it would be a panic to have more than needed in
  // for declaration, e.g. `decl foo(let arg: x : T)`.
  const optional<vector<ReferenceStatement>> arguments;

  // Only applicable to movings (e.g. `moveimpl foo() [to bar]`)
  // and re-implementations (e.g. `reimpl foo() [as bar];`).
  const optional<pair<Token::Keyword, Token::ID>> new_id;

  // There may be multiple foralls,
  // e.g. `forall T<U> ~ V, W forall U`.
  const optional<
      vector<pair<Token::Keyword, vector<ComplexTypeReference>>>>
      foralls;

  // An optional `where` delayed (non-final) macro.
  const optional<Token::Macro> where;

  // Only set for (re)implementations, excluding native ones.
  const optional<vector<Expression>> body;
};

struct UnaryTypeOperation : Node {
  enum Operator {
    Not, // E.g. `!T`
  };

  // NOTE: `operator` is a keyword in C++.
  const Token::Operator op;

  const unique_ptr<TypeExpression> operand;

  Operator get_op() const;
};

struct BinaryTypeOperation : Node {
  enum Operator {
    And, // E.g. `A && B`
    Or,  // E.g. `A || B`
    Xor, // E.g. `A ^ B`
  };

  // NOTE: `operator` is a keyword in C++.
  const Token::Operator op;

  const unique_ptr<TypeExpression> left_operand;
  const unique_ptr<TypeExpression> right_operand;

  Operator get_op() const;
};

struct TypeExpression : Node {
  const optional<pair<Token::Control, Token::Control>> brackets;

  const variant<
      unique_ptr<UnaryTypeOperation>,
      unique_ptr<BinaryTypeOperation>,
      shared_ptr<RealTypeReference>>
      expr;
};

struct ComplexTypeReference : Node {
  optional<RealTypeReference> real;
  optional<TypeExpression> imaginary;
};

} // namespace SST
} // namespace Compiler
} // namespace Onyx
