#pragma once

#include "./token.hpp"
#include <map>
#include <set>
#include <variant>

using namespace std;

namespace Onyx {
namespace Compiler {
namespace AST {
struct Node {
  virtual ~Node() {}
  virtual void dump(ostream *, unsigned short = 0);
};

struct Root : Node {};

// Something declared in a namespace, which includes
// functions, variables and other namespaces.
struct Declaration : Node {
  void dump(ostream *, unsigned short tab = 0);
};

struct FunctionDefinition;

struct Namespace : Declaration {
  wstring name;
  shared_ptr<Namespace> parent_namespace;

  set<shared_ptr<FunctionDefinition>> functions;
  set<shared_ptr<Namespace>> namespaces;
  // set<shared_ptr<VariableDeclaration>> variables;
  // set<shared_ptr<ConstantDeclaration>> constants;

  void dump(ostream *, unsigned short tab = 0);
};

struct Expression : Node {};

struct ID : Expression {
  shared_ptr<Token::Value> value;
};

struct Literal : Expression {};

// String literal is a continuation of strings
// mixed with interpolation expressions.
struct StringLiteral : Literal {
  vector<variant<shared_ptr<Token::Value>, shared_ptr<Expression>>>
      values;
};

struct NumericLiteral : Literal {};

struct Body : Node {
  set<shared_ptr<Expression>> expressions;
  void dump(ostream *, unsigned short tab = 0);
};

struct Arguments {
  set<shared_ptr<Expression>> ordered_arguments;
  map<string, shared_ptr<Expression>> named_arguments;
};

// Annotation "usage".
struct AnnotationApplication : Node {
  shared_ptr<Token::Value> id;
  Arguments args;
};

// `..foo` is an example of a regular splat, while `**foo` is a named
// one.
struct Splat : Expression {
  bool is_named;
  shared_ptr<Token::Value> id;
};

// Binary operations involve left and right hand
// expressions and the operator between them.
struct Binop : Expression {
  shared_ptr<Expression> lhx;
  shared_ptr<Token::Value> op;
  shared_ptr<Expression> rhx;
};

// Unary operation has the operator and the expression.
struct Unop : Expression {
  shared_ptr<Token::Value> op;
  shared_ptr<Expression> expr;
};

struct Call : Expression {
  shared_ptr<Token::Value> modifiers;
  shared_ptr<Expression> caller;
  shared_ptr<Token::Value> callee;
  Arguments args;
};

struct FunctionArgumentDeclaration : Node {
  set<shared_ptr<AnnotationApplication>> annotations;
  bool is_const;

  enum { Common, Vargs, Kwargs } type;

  shared_ptr<Token::Value> alias;
  shared_ptr<Token::Value> name;

  shared_ptr<Expression> restriction;
  shared_ptr<Expression> default_value;

  void dump(ostream *, unsigned short tab = 0);
};

struct FunctionPrototype : Node {
  set<shared_ptr<AnnotationApplication>> annotations;
  set<shared_ptr<Token::Value>> modifiers;
  shared_ptr<Token::Value> name;
  set<shared_ptr<FunctionArgumentDeclaration>> args;

  void dump(ostream *, unsigned short tab = 0);
};

// It's only applicable to abstract functions.
struct FunctionDeclaration : Declaration {
  shared_ptr<FunctionPrototype> prototype;
};

struct FunctionDefinition : Declaration {
  shared_ptr<Namespace> parent_namespace;
  shared_ptr<FunctionPrototype> prototype;
  shared_ptr<Body> body;

  void dump(ostream *, unsigned short tab = 0) override;
};

// Module is a namespace with instance and abstract methods.
struct Module : Namespace {
  set<shared_ptr<FunctionDeclaration>> function_declarations;
};

// Object is an instantiate-able module.
struct Object : Module {};

struct Primitive : Object {};

struct Struct : Object {};

struct Class : Object {};

struct Enum : Declaration {};

struct Annotation : Declaration {};

static wstring tabs(unsigned short tab) {
  wstring buffer;

  for (unsigned short i = 0; i < tab; i++) {
    buffer.append(L"  ");
  }

  return buffer;
}
}; // namespace AST
} // namespace Compiler
} // namespace Onyx
