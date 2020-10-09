#pragma once

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <tuple>
#include <variant>
#include <vector>

#include "./cst.hpp"

using namespace std;

namespace Onyx {
namespace Compiler {
// The program-wide Abstract Syntax Tree.
// It is enriched constantly throughout the compilation process.
//
// NOTE: When a smart pointer is not `optional`,
// that means that it MUST be set.
//
// TODO: Should a type defined in a `forall` clause or a generic
// argument definition reference a type declaration?
//
// NOTE: Due to the neccessity to cross-reference nodes,
// the node declarations are separate from definitions.
//
// TODO: The AST is enriched ASAP, i.e. once some statement is
// complete in an SST.
namespace AST {
struct Node;

// A compile-time statement, e.g. a type declaration.
struct ComptimeStatement;

// A callable subroutine statement.
// It may be a declaration, implementation or definition.
// It may be a procedure or a generator.
struct SubroutineStatement;

// A concrete subroutine specialization.
struct SubroutineSpecialization;

// A reference declaration used in argument declarations, e.g.
// `decl foo(x: SBin32)`: you can only declare
// its "outer" name, i.e. an alias.
struct ReferenceDeclaration;

// A reference definition, e.g. `let x : SBin32`.
struct ReferenceDefinition;

// A passing of a reference as an argument, e.g.
// `foo: bar` or `[0]: id` or simply `id`.
struct ReferencePass;

// A macro definition, e.g. `macro @foo;`.
// Note that a macro can not be declared.
struct MacroDefinition;

// A type declaration, implementation,
// definition or reopening.
struct TypeStatement;

// A distinct type specialization.
struct TypeSpecialization;

// A complex type referenced from code, e.g. `T<U>~V<W>`.
struct TypeReference;

// A type expression, e.g. `(A && B)`.
// It consists of logical operands
// and may be nested with parentheses.
struct TypeExpression;

// An unary operation used in type
// expressions, e.g. `!T` or simple `T`.
struct TypeUnaryOperation;

// A binary operation used in type
// expressions, e.g. `A && B`.
struct TypeBinaryOperation;

// A runtime expression.
struct Expression;

// A runtime statement, e.g. `if`.
struct RuntimeStatement;

enum Visibility {
  VisibilityPublic,
  VisibilityProtected,
  VisibilityPrivate
};

enum Safety { SafetyThreadsafe, SafetyFragile, SafetyUnsafe };

// Declaration scope is different from pointer scopes.
enum DeclarationScope {
  DeclarationScopeLocal,
  DeclarationScopeInstance,
  DeclarationScopeStatic
};

enum Scope {
  ScopeStatic,
  ScopeInstance,
  ScopeCaller,
  ScopeLocal,
  ScopeTemporal
};

// An `alias` statement.
// May be a part of a `distinct alias` type statement.
struct Alias;

// An identifier path reference without arguments.
// For example, `::Foo::Bar<T>:baz` or simple `foo`.
struct Path;

static string tabs(unsigned short tab);

} // namespace AST
} // namespace Compiler
} // namespace Onyx

namespace Onyx {
namespace Compiler {
namespace AST {

struct Node {
  virtual ~Node() {}
  virtual void dump(ostream *output, unsigned short tab = 0);
};

struct ComptimeStatement : Node {};

struct ComplexTypeReference;

// An `Alias` node is created for each source.
//
// When an alias is looked up, it shall match the argument
// declarations.
//
// ```nx
// alias foo(x: SBin32) = bar(x)
//
// # Looks up for `foo` with matching restriction.
// # When found, calls `bar`, maybe specializing.
// foo(1)
//
// # Would not match the alias.
// # foo(0.5) # Panic!
// ```
struct Alias : ComptimeStatement {
  // The containing, parent statement of this alias.
  // May only be a subroutine or type statement.
  const shared_ptr<ComptimeStatement> parent;

  // The original `alias` statement.
  const shared_ptr<SST::Alias> statement;

  // An ID token offset within the `alias`
  // statement, beginning from zero.
  const unsigned char id_offset;

  // Concrete argument restrictions of this alias.
  const optional<vector<ReferenceDeclaration>> arguments;

  // Whether does the alias contains matchall (`*`)
  // clause in the end of the arguments list.
  const bool matchall;

  // The target statement of this alias, i.e. what it aliases to.
  // May only be a subroutine or type declaration or alias statement.
  const shared_ptr<ComptimeStatement> target;

  // Concrete argument passes to the target.
  const optional<vector<ReferencePass>> target_arguments;

  // Whether does the target contains matchall (`*`) in the end.
  const bool target_matchall;

  // An `alias` statement may contain multiple `forall` clauses.
  //
  // ```nx
  // alias foo(x: T) = bar(x) forall T~Numeric
  // alias foo(x: ~Numeric) = bar(x) # Ditto
  // ```
  const vector<vector<ComplexTypeReference>> forall;
};

struct TypeStatement : ComptimeStatement {
  // A type may be declared within another type or
  // procedure declaration or specialization.
  // Would be empty for the top-level namespace.
  const optional<variant<
      shared_ptr<SubroutineStatement>,
      shared_ptr<SubroutineSpecialization>,
      shared_ptr<TypeStatement>,
      shared_ptr<TypeSpecialization>>>
      parent;

  const optional<vector<unique_ptr<TypeSpecialization>>> annotations;
  const optional<vector<shared_ptr<Token::Text>>> documentation;

  const optional<vector<shared_ptr<Token::Keyword>>>
      modifier_keywords;

  // Points to the declaration statement keyword, e.g. `namespace`
  // or `distinct`. Would be empty for the top-level namespace.
  // Points to `distrinct` iff `kind` is `Distinct`.
  const optional<shared_ptr<Token::Keyword>> statement_keyword;

  // Would be empty for the top-level namespace.
  const optional<shared_ptr<Token::ID>> id;
  const optional<vector<ArgumentDefinition>> arguments;

  // Replaces `id` and `arguments`.
  // Only set iff `kind` is `Distinct`.
  const optional<shared_ptr<Alias>> alias;

  // An optional conditional `where` macro.
  const optional<shared_ptr<Token::Text>> where;

  vector<shared_ptr<SubroutineStatement>> subroutine_statements;
  vector<shared_ptr<TypeStatement>> type_statements;
  vector<shared_ptr<ReferenceDefinition>> reference_definitions;
  vector<shared_ptr<MacroDefinition>> macro_definitions;
  vector<unique_ptr<Macro>> delayed_macros;
  vector<unique_ptr<Macro>> final_macros;

  enum Kind {
    Namespace,
    Trait,
    Struct,
    Class,
    Enum,
    Flag,
    Distinct,
  };

  const Kind kind;

  // The following properties may be inferred.
  //

  optional<Visibility> visibility;

  // Would be true for `compl`, false for `incompl`.
  // Only applicable to object types.
  optional<bool> is_complete;

  // Would be true for `mut`, false for `const`.
  // Only applicable to classes.
  optional<bool> is_mutable;

  // Would be true for `abstract`, false for not specified.
  // Only applicable to structs and classes.
  bool is_abstract;

  // If this is an implementation, points to
  // declaration, which may be not resolved yet.
  // If this is a definition, points to itself.
  optional<shared_ptr<TypeStatement>> declaration;

  void dump(ostream *, unsigned short tab = 0);
};

struct TypeSpecialization : Node {
  // Where did it specialize for the first time?
  const shared_ptr<Token::ID> triggered_id;

  // The original type statement.
  const shared_ptr<TypeStatement> origin;

  // Arguments specific to this specialization.
  const optional<vector<unique_ptr<ArgumentPassing>>> arguments;

  // A type specialization may state new
  // members, excluding delayed macros.
  //

  vector<shared_ptr<SubroutineStatement>> subroutine_statements;
  vector<shared_ptr<TypeStatement>> type_statements;
  vector<shared_ptr<ReferenceDefinition>> reference_definitions;
  vector<shared_ptr<MacroDefinition>> macro_definitions;
  vector<shared_ptr<Macro>> final_macros;
};

struct TypeReference : Node {
  Location location;

  // Empty value means no explicit real type (i.e. to be inferred).
  optional<shared_ptr<TypeSpecialization>> real;

  // Empty value means no explicit imaginary type.
  optional<shared_ptr<TypeExpression>> imaginary;
};

struct TypeUnaryOperation : Node {
  Location location;

  enum Operator {
    Not, // E.g. `!T`
  };

  // NOTE: `operator` is a keyword in C++.
  const Operator op;

  unique_ptr<TypeExpression> operand;
};

struct TypeBinaryOperation : Node {
  Location location;

  enum Operator {
    And, // E.g. `A && B`
    Or,  // E.g. `A || B`
    Xor, // E.g. `A ^ B`
  };

  // NOTE: `operator` is a keyword in C++.
  const Operator op;

  unique_ptr<TypeExpression> left_operand;
  unique_ptr<TypeExpression> right_operand;
};

struct TypeExpression : Node {
  Location location;

  variant<
      unique_ptr<TypeUnaryOperation>,
      unique_ptr<TypeBinaryOperation>,
      shared_ptr<TypeSpecialization>>
      expr;
};

struct ReferenceDefinition : ComptimeStatement {
  const variant<
      shared_ptr<TypeStatement>,
      shared_ptr<TypeSpecialization>,
      shared_ptr<SubroutineStatement>,
      shared_ptr<SubroutineSpecialization>>
      parent;

  // Points to `let`, `final`, `get` or `set` keyword.
  shared_ptr<Token::Keyword> statement_keyword;

  const Path path;
  optional<shared_ptr<TypeReference>> type_restriction;
  const shared_ptr<Expression> value;

  // Excludes the statement keyword.
  const optional<vector<shared_ptr<Token::Keyword>>>
      modifier_keywords;

  optional<vector<unique_ptr<TypeSpecialization>>> annotations;
  const optional<vector<shared_ptr<Token::Text>>> documentation;

  optional<Visibility> visibility;
  optional<Scope> scope;
  optional<Safety> safety;
};

struct MacroDefinition : ComptimeStatement {
  // A macro may be defined within a type or a subroutine.
  variant<shared_ptr<SubroutineStatement>, shared_ptr<TypeStatement>>
      parent;

  const shared_ptr<Token::ID> id;

  // Can have visibility and safety modifers.
  vector<shared_ptr<Token::Keyword>> modifier_keywords;

  // NOTE: A macro can not have annotations.

  const optional<vector<shared_ptr<Token::Text>>> documentation;

  // A macro body. Can be empty.
  const optional<shared_ptr<Token::Text>> body;

  optional<Visibility> visibility;

  // A macro may have its own safety,
  // which is required upon calling it.
  optional<Safety> safety;
};

struct SubroutineStatement : Member {
  // A subroutine statement may even be contained
  // within a subroutine statement. In other words,
  // a function may be declared within a function.
  const variant<
      shared_ptr<SubroutineStatement>,
      shared_ptr<TypeStatement>>
      parent;

  vector<shared_ptr<TypeReference>> annotations;
  const optional<vector<shared_ptr<Token::Text>>> documentation;

  const vector<shared_ptr<Token::Keyword>> modifiers;

  enum StatementKind {
    Declaration,        // `decl foo()`
    Implementation,     // `impl foo();`
    Definition,         // `def foo();`
    MoveImplementation, // `moveimpl foo() to bar`
    ReImplementation,   // `reimpl foo();`, `reimpl foo() as bar;`
    UnDeclaration,      // `undecl foo()`
  };

  // See `StatementKind`.
  const shared_ptr<Token::Keyword> statement_keyword;

  enum SubroutineKind {
    Procedure, // `decl proc foo()`, default
    Generator, // `decl gen foo()`, inferred if args contain a block
  };

  // See `SubroutineKind`.
  const optional<shared_ptr<Token::Keyword>> kind_keyword;

  // The subroutine identifier.
  const shared_ptr<Token::ID> id;

  // Optional argument definitions.
  const optional<vector<shared_ptr<ArgumentDefinition>>> arguments;

  // Only applicable to `moveimpl foo() to bar`
  // and `reimpl foo() as bar;`.
  const optional<
      tuple<shared_ptr<Token::Keyword>, shared_ptr<Token::ID>>>
      new_id;

  // There may be multiple foralls,
  // e.g. `forall T<U> ~ V, W forall U`.
  const optional<vector<vector<TypeReference>>> forall;

  // An optional `where` delayed macro.
  const optional<Token::Text> where;

  // Only set if it is `impl`, `def` or `reimpl`.
  const optional<vector<Expression>> body;

  // If this is an `impl`, `moveimpl`, `reimpl`, `undecl`, points to
  // declaration, which may be not resolved yet. If this is `def`,
  // points to itself. Empty if this is `decl`.
  optional<shared_ptr<SubroutineStatement>> declaration;

  // May-be-inferred resulting attributes.
  //

  optional<Visibility> visibility;
  optional<DeclarationScope> scope;
  optional<bool> mutating; // True for `mut`, false for `const`
  optional<Safety> safety;
};

// TODO: Vargs, e.g. `*arg : T`.
// TODO: `final`, `const`. These all are just references?
struct ArgumentDefinition : Node {
  variant<shared_ptr<TypeStatement>, shared_ptr<SubroutineStatement>>
      context;

  // E.g. `foo: bar : T` or `[0]: bar : T`.
  const optional<variant<shared_ptr<Token::ID>, unsigned short>>
      alias;

  // May be anonymous, e.g. `_ : T`.
  const optional<shared_ptr<Token::ID>> name;

  const optional<shared_ptr<TypeReference>> type_restriction;
  const optional<shared_ptr<Expression>> default_value;

  vector<shared_ptr<TypeReference>> annotations;
};

// Binary operations involve left and right hand
// expressions and the operator between them.
struct Binop : Expression {
  const shared_ptr<Expression> lhx;
  const shared_ptr<Token::Operator> op;
  const shared_ptr<Expression> rhx;
};

// Unary operation has the operator and the expression.
struct Unop : Expression {
  const shared_ptr<Token::Operator> op;
  const shared_ptr<Expression> expr;
};

struct Call : Expression {
  shared_ptr<Token::Keyword> modifiers;
  shared_ptr<Expression> caller;
  shared_ptr<Token::Value> callee;
  Arguments args;
};
}; // namespace AST
} // namespace Compiler
} // namespace Onyx
