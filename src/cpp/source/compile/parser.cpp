#include <experimental/filesystem>
#include <iostream>

#include "./parser.hpp"

namespace fs = experimental::filesystem;

namespace Onyx {
Parser::Error::Error(shared_ptr<Token::Base> token, const wstring reason) :
    token(token),
    reason(reason) {}

Parser::CompilationRequest::CompilationRequest(bool is_import) :
    is_import(is_import) {}

Parser::Parser(
    Lexer *lexer,
    vector<shared_ptr<Token::Base>> *tokens,
    shared_ptr<SAST::Node> sast_root,
    mutex *sast_mutex) :
    _tokens(tokens),
    _lexer(lexer),
    _sast_mutex(sast_mutex) {
  _sast_stack.push(sast_root);
  lex();
}

optional<Parser::CompilationRequest> Parser::parse() {
  // while (!is<Token::Eof>()) {
  //   if (auto id = as<Token::ID>()) {
  //     if (id->value == L"require") {
  //       // The require statement defines a number of paths to require.
  //       // The path is relative to the current file.
  //       // If omitted, the `.nx` extension is implied.
  //       // Paths to require can be comma-separated.
  //       // Some implementation-defined special locations are supported,
  //       // such as `&ext/`.
  //       //
  //       // ```
  //       // require "./relative"
  //       // require "/absolute", "../relative"
  //       // require "&ext/opencl"
  //       // ```
  //       //

  //       CompilationRequest request(false);
  //       bool is_comma = false;

  //       lex();

  //       while (true) {
  //         if (auto string = as<Token::StringLiteral>()) {
  //           request.paths.push_back(fs::path(string->value));
  //           lex();
  //           is_comma = false;
  //         } else if (request.paths.size() == 0) {
  //           throw Error(id, L"Expected paths to require");
  //         } else if (is_exact(L",")) {
  //           if (is_comma)
  //             throw Error(_token, L"Unexpected comma");

  //           is_comma = true;
  //           lex();
  //         } else if (is_terminator()) {
  //           lex();

  //           if (!is_comma)
  //             return request;
  //         } else {
  //           throw Error(_token, L"Unexpected token");
  //         }
  //       }
  //     } else if (id->value == L"def") {
  //       // Function definition
  //       //

  //       lex(); // Consume the "def" keyword

  //       auto ns = dynamic_pointer_cast<SAST::Namespace>(_sast_stack.top());

  //       if (!ns)
  //         throw Error(_token, L"Can only define functions within
  //         namespaces");

  //       if (!is<Token::ID>())
  //         throw Error(_token, L"Expected function name");

  //       auto prototype = make_shared<SAST::FunctionPrototype>();
  //       prototype->name.emplace<shared_ptr<Token::ID>>(as<Token::ID>());

  //       auto body = make_shared<SAST::Body>();

  //       auto definition = make_shared<SAST::FunctionDefinition>();
  //       definition->parent_namespace = ns;
  //       definition->body = body;
  //       definition->prototype = prototype;

  //       {
  //         lock_guard<mutex> lock(*_sast_mutex);
  //         ns->functions.insert(definition);
  //       }

  //       lex(); // Consume the function name

  //       _sast_stack.push(definition); // We're now within
  //                                     // the function definition

  //       if (is_exact(L"(")) {
  //         lex(); // Consume the opening bracket

  //         while (!is_exact(L")")) {
  //           // A function argument may have a name and an alias
  //           //

  //           auto alias = as<Token::ID>();

  //           if (!alias)
  //             throw Error(_token, L"Expected argument name");

  //           auto decl = make_shared<SAST::FunctionArgumentDeclaration>();
  //           prototype->args.insert(decl);

  //           lex();
  //           auto name = as<Token::ID>();

  //           if (name) {
  //             decl->alias = alias;
  //             decl->name = name;
  //             lex(); // Consume the name
  //           } else
  //             decl->name = alias;

  //           if (is_exact(L":")) {
  //             // Argument has a restriction,
  //             // which must be a type expression
  //             //

  //             lex(); // Consume the colon
  //             decl->restriction = parse_expression();
  //           }

  //           if (is_exact(L"=")) {
  //             // Argument has a default value,
  //             // which must be an expression
  //             //

  //             lex(); // Consume the "=" symbol
  //             decl->default_value = parse_expression();
  //           }

  //           if (is_exact(L","))
  //             lex(); // Consume the comma
  //         }

  //         lex(); // Consume the closing bracket
  //       }

  //       skip_newlines();

  //       while (!is_end()) {
  //         if (is<Token::Eof>())
  //           throw Error(_token, L"Expected end");

  //         body->expressions.insert(parse_expression());
  //       }

  //       lex(); // Consume the "end" keyword

  //       _sast_stack.pop(); // Exit the function definition
  //     } else {
  //       throw Error(_token, L"Unexpected token");
  //     }
  //   } else if (is<Token::Newline>()) {
  //     lex();
  //     continue;
  //   } else {
  //     throw Error(_token, L"Unexpected token");
  //   }
  // }

  // return nullopt;
}

shared_ptr<SAST::Expression> Parser::parse_expression() {
  // if (auto id = as<Token::ID>()) {
  //   lex();

  //   auto node = make_shared<SAST::ID>();
  //   node->id = id;

  //   if (is_terminator())
  //     return node;

  //   if (auto op = as<Token::Op>()) {
  //     lex();

  //     auto binop = make_shared<SAST::Binop>();

  //     binop->lhx = node;
  //     binop->op = op;

  //     binop->rhx = parse_expression();
  //     if (!binop->rhx)
  //       throw Error(op, L"Expected right-hand expression");

  //     return binop;
  //   } else if (is<Token::Access>()) {
  //     lex();

  //     auto callee = as<Token::ID>();
  //     if (!callee)
  //       throw Error(_token, L"Expected identifier");
  //   }
  // } else if (auto op = as<Token::Op>()) {
  //   auto unop = make_shared<SAST::Unop>();
  //   unop->op = op;

  //   unop->expr = parse_expression();
  //   if (!unop->expr)
  //     throw Error(op, L"Expected expression");

  //   return unop;
  // }
}

void Parser::lex() {
  _token = _lexer->lex();
  debug_token();
};

void Parser::debug_token() {
  if (auto id = as<Token::Value>())
    wcerr << id->pretty_kind() << '\t' << id->value << '\n';
  else if (auto ctrl = as<Token::Control>())
    wcerr << "CONTROL\t" << ctrl->pretty_source() << '\n';
  else
    wcerr << "UNKNOWN\t\n";
}

template <class T> shared_ptr<T> Parser::as() {
  return dynamic_pointer_cast<T>(_token);
}

template <class T> bool Parser::is() { return (!!(as<T>())); }

bool Parser::is_exact(wstring value) {
  // if (auto tok = as<Token::Other>())
  //   return tok->value == value;
  // else
  //   return false;
}

bool Parser::is_end() {
  // if (auto id = as<Token::ID>())
  //   return id->value == L"end";
  // else
  //   return false;
}

void Parser::skip_newlines() {
  while (is_newline())
    lex();
}

bool Parser::is_terminator() {
  return is_newline() || is_exact(L";") || is_eof();
}

bool Parser::is_eof() {
  if (auto ctrl = as<Token::Control>())
    return ctrl->kind == Token::Control::Eof;
  else
    return false;
}

bool Parser::is_newline() {
  if (auto ctrl = as<Token::Control>())
    return ctrl->kind == Token::Control::Newline;
  else
    return false;
}

// class Parser {
//   Lexer *_lexer;
//   shared_ptr<Token::Base> _token;
//   stack<SAST::Node *> _sast_stack;

// public:
//   struct Error {
//     shared_ptr<Token::Base> token;
//     const wstring reason;

//     Error(shared_ptr<Token::Base> token, const wstring reason) :
//     token(token), reason(reason) {}
//   };

//   struct CompilationRequest {
//     bool is_import;
//     vector<fs::path> paths;

//     CompilationRequest(bool is_import) : is_import(is_import) {}
//   };

//   Parser(Lexer *lexer, SAST::Node *sast_root) : _lexer(lexer) {
//     _sast_stack.push(sast_root);
//     lex();
//   }

//   optional<CompilationRequest> parse() {
//     while (!is<Token::Eof>()) {
//       if (auto id = as<Token::ID>()) {
//         if (id->value == L"require") {
//           // The require statement defines a number of paths to require.
//           // The path is relative to the current file.
//           // If omitted, the `.nx` extension is implied.
//           // Paths to require can be comma-separated.
//           // Some implementation-defined special locations are supported,
//           // such as `&ext/`.
//           //
//           // ```
//           // require "./relative"
//           // require "/absolute", "../relative"
//           // require "&ext/opencl"
//           // ```
//           //

//           CompilationRequest request(false);
//           bool is_comma = false;

//           lex();

//           while (true) {
//             if (auto string = as<Token::StringLiteral>()) {
//               request.paths.push_back(fs::path(string->value));
//               lex();
//               is_comma = false;
//             } else if (request.paths.size() == 0) {
//               throw Error(id, L"Expected paths to require");
//             } else if (as<Token::Comma>()) {
//               if (is_comma)
//                 throw Error(_token, L"Unexpected comma");

//               is_comma = true;
//               lex();
//             } else if (is_terminator()) {
//               lex();

//               if (!is_comma)
//                 return request;
//             } else {
//               throw Error(_token, L"Unexpected token");
//             }
//           }
//         } else {
//           throw Error(_token, L"Unexpected token");
//         }
//       } else if (is<Token::Newline>()) {
//         lex();
//         continue;
//       } else {
//         throw Error(_token, L"Unexpected token");
//       }
//     }

//     return nullopt;
//   }

// private:
//   void lex() { _token = _lexer->lex(); };

//   template <class T> shared_ptr<T> as() { return
//   dynamic_pointer_cast<T>(_token); } template <class T> bool is() { return
//   (!!(as<T>())); }

//   // Expressions are terminated with either newlines, semicolon or EOF.
//   bool is_terminator() { return is<Token::Newline>() || is<Token::Semi>()
//   || is<Token::Eof>(); }
// };
} // namespace Onyx
