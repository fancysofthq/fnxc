#include <memory>
#include <vector>

#include "./token.cpp"

using namespace std;

namespace Onyx {
namespace AST {
struct Node {
  virtual ~Node() {}
};

struct Root : Node {};

struct TypeReference : Node {};

struct TypeExpression : Node {};

struct Expression : Node {};

struct Body : Node {
  vector<unique_ptr<Expression>> expressions;
};

struct Branch : Node {
  unique_ptr<Expression> condition;
  unique_ptr<Body> body;
};

enum BranchType { If, Unless, Switch, Case, Select };

struct BranchStatement : Expression {
  BranchType type;
  vector<unique_ptr<Branch>> branches;
};

enum LoopType { While, Until };

struct LoopStatement : Expression {
  LoopType type;
  unique_ptr<Expression> condition;
  unique_ptr<Body> body;
};

struct Call : Expression {
  shared_ptr<Expression> caller;
  vector<unique_ptr<Token::Keyword>> modifiers;
  shared_ptr<FunctionPrototype> callee;
  vector<CallArgument> args;
  unique_ptr<CallBlock> block;
};

struct CDeclaration : Node {};

struct CFunctionPrototype : CDeclaration {
  string name_literal;              // Set for imported functions
  unique_ptr<Token::ID> name_token; // Set for exported or external functions
};

struct CArg {
  bool is_const;
  const string name;
  const string type;
};

struct ExternalFunctionDeclaration : Declaration {
  const string name;
  const vector<CArg> args;
  const string return_type;
};

struct ExportedFunctionDefinition : Declaration {
  const ExternalFunctionDeclaration prototype;
  const Body body;
};

struct CCall : Expression {
  string undefined_name;
  shared_ptr<CFunctionPrototype> callee;
  vector<CallArgument> args;
  unique_ptr<CallBlock> block;
};

// May be literal or type (`literal | ([".." | "**"], type_ref)`)
struct ConstantExpression : Node {};

struct Declaration : Node {};

struct MacroExpression : Node {};

// Generic argument declaration in a type or `forall` statement.
//
// ```onyx
// module Enumerable(Element E out T)
// end
//
// class Proc(Arguments in ..T, Return out U)
// end
//
// primitive Slice(Type T, Size S : SizeLiteral)
// end
//
// class Hash(K, V, H : Hasher = DefaultHasher)
// end
//
// def sum(a : T, b : T) forall T : Number
// end
// ```
struct GenericArgumentDeclaration : Node {
  unsigned variance; // 0: undefined, 1: out (co-), 2: in (contra-)

  shared_ptr<Token::Const> alias; // Optional
  shared_ptr<Token::Const> name;

  unsigned splat; // 0: undefined, 1: tuple, 2: named tuple

  shared_ptr<TypeExpression> restriction;       // Optional
  shared_ptr<ConstantExpression> default_value; // Optional
};

// A code block restriction; *from* turns into *to*.
//
// ```ebnf
// block_arg_decl = [type_decl], "~>", [type_decl];
// ```
//
// ```onyx
// def foo(proc : T | U ~> V)
// end
// ```
struct BlockRestriction : Node {
  unique_ptr<TypeExpression> from;
  unique_ptr<TypeExpression> to;
};

// Onyx function argument.
//
// ```ebnf
// function_arg_decl = [id], id, [":", type_expr | block_arg_decl], ["=", expr];
// function_args_decl = function_arg_decl, {",", function_arg_decl}, [block_arg_decl];
// ```
struct FunctionArgumentDeclaration : Node {
  unique_ptr<Token::ID> alias; // Optional
  unique_ptr<Token::ID> name;

  // Either is optional, but not at the same time
  unique_ptr<TypeExpression> type_restriction;
  unique_ptr<BlockRestriction> block_restriction;

  unique_ptr<Expression> default_value; // Optional
};

// Onyx function prototype.
//
// ```ebnf
// function_proto =
//   ["private" | "protected"],
//   ["static"],
//   ["safe" | "volatile" | "unsafe"],
//   ["const"],
//   "def", (
//     function_id | op, ["(", function_args_decl, ")"]
//   ) | (
//     "[", function_args_decl, "]", ["=", "(", function_args_decl, ")"]
//   ),
//   [":", type_expr];
// ```
struct FunctionPrototype : Node {
  vector<unique_ptr<Token::ID>> modifiers;
  unique_ptr<Token::ID> name;
  vector<unique_ptr<FunctionArgumentDeclaration>> args;
  vector<unique_ptr<FunctionArgumentDeclaration>> indexable_args;
  unique_ptr<FunctionArgumentDeclaration> indexable_value;
  unique_ptr<TypeExpression> return_restriction;
};

// Any Onyx function declaration. Only possible for abstract functions.
//
// ```ebnf
// function_decl = "abstract", function_proto, ["forall", generic_arg_decl];
// ```
struct FunctionDeclaration : Declaration {
  bool is_abstract;
  unique_ptr<FunctionPrototype> prototype;
  vector<unique_ptr<GenericArgumentDeclaration>> forall;
};

// An Onyx function definition.
//
// ```ebnf
// function_def =
//   function_proto,
//   ["forall", generic_arg_decl, ["where", macros]],
//   {expr, newexpr}, "end";
// ```
struct FunctionDefinition : Declaration {
  unique_ptr<FunctionPrototype> prototype;
  vector<unique_ptr<GenericArgumentDeclaration>> forall;
  unique_ptr<MacroExpression> where;
  vector<Expression> body;
};

// ```ebnf
// intrinsic_def =
//   ["private" | "protected"],
//   "macro",
//   "@", nospace, {id, ".", nospace}, function_id,
//   ["(", function_arg_decl, ")"],
//   ["where", macros],
//   {expr, newexpr},
//   "end";
// ```
struct IntrinsicDefinition : Declaration {
  vector<unique_ptr<Token::ID>> modifiers;
  unique_ptr<Token::ID> name;
  vector<unique_ptr<FunctionArgumentDeclaration>> args;
  unique_ptr<MacroExpression> where;
  vector<unique_ptr<Expression>> body;
};

// ```ebnf
// variable_decl =
//   ["private" | "protected"],
//   ["static"],
//   ["safe", "volatile", "unsafe"],
//   ["var" | "getter"], id,
//   [":", type_expr],
//   ["=", expr];
//
// (* Constants can't be static, and require their values to be set*)
// constant_decl =
//   ["private" | "protected"],
//   "const", id,
//   [":", type_expr],
//   "=", expr;
// ```
struct VariableDeclaration : Declaration {
  unsigned visibility_modifier : 2; // 0: unknown, 1: private, 2: protected
  bool is_static;
  unsigned safety_modifier : 2; // 0: unknown, 1: safe, 2: volatile, 3: unsafe
  bool is_getter;
  unique_ptr<TypeExpression> restriction;
  unique_ptr<Expression> value;
};

// Constants can't be static, and require their values to be set.
//
// ```ebnf
// constant_decl =
//   ["private" | "protected"],
//   "const", id,
//   [":", type_expr],
//   "=", expr;
// ```
struct ConstantDeclaration : Declaration {
  unsigned visibility_modifier : 2; // 0: unknown, 1: private, 2: protected
  unique_ptr<TypeExpression> restriction;
  unique_ptr<Expression> value;
};

enum TypeType {
  Unknown, // Valid for implicit namespaces
  Namespace,
  Module,
  Primitive,
  Struct,
  Class
};

struct TypeDefinition : Declaration {
  TypeType type;
  shared_ptr<Token::ID> name;
  vector<shared_ptr<GenericArgumentDeclaration>> generic_args;
  vector<shared_ptr<TypeReference>> derives;
  vector<shared_ptr<Declaration>> declarations;
};
} // namespace AST
} // namespace Onyx
