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
    Lexer *lexer, shared_ptr<SAST::Node> sast_root, mutex *sast_mutex) :
    _lexer(lexer),
    _sast_mutex(sast_mutex) {
  _sast_stack.push(sast_root);
  lex();
}

optional<Parser::CompilationRequest> Parser::parse() {
  while (!is<Token::Eof>()) {
    if (auto id = as<Token::ID>()) {
      if (id->value == L"require") {
        // The require statement defines a number of paths to require.
        // The path is relative to the current file.
        // If omitted, the `.nx` extension is implied.
        // Paths to require can be comma-separated.
        // Some implementation-defined special locations are supported,
        // such as `&ext/`.
        //
        // ```
        // require "./relative"
        // require "/absolute", "../relative"
        // require "&ext/opencl"
        // ```
        //

        CompilationRequest request(false);
        bool is_comma = false;

        lex();

        while (true) {
          if (auto string = as<Token::StringLiteral>()) {
            request.paths.push_back(fs::path(string->value));
            lex();
            is_comma = false;
          } else if (request.paths.size() == 0) {
            throw Error(id, L"Expected paths to require");
          } else if (is_exact(L",")) {
            if (is_comma)
              throw Error(_token, L"Unexpected comma");

            is_comma = true;
            lex();
          } else if (is_terminator()) {
            lex();

            if (!is_comma)
              return request;
          } else {
            throw Error(_token, L"Unexpected token");
          }
        }
      } else if (id->value == L"def") {
        // Function definition
        //

        lex(); // Consume the "def" keyword

        auto ns = dynamic_pointer_cast<SAST::Namespace>(_sast_stack.top());

        if (!ns)
          throw Error(_token, L"Can only define functions within namespaces");

        if (!is<Token::ID>())
          throw Error(_token, L"Expected function name");

        auto prototype = make_shared<SAST::FunctionPrototype>();
        prototype->name.emplace<shared_ptr<Token::ID>>(as<Token::ID>());

        auto body = make_shared<SAST::Body>();

        auto definition = make_shared<SAST::FunctionDefinition>();
        definition->parent_namespace = ns;
        definition->body = body;
        definition->prototype = prototype;

        {
          lock_guard<mutex> lock(*_sast_mutex);
          ns->functions.insert(definition);
        }

        lex(); // Consume the function name

        _sast_stack.push(definition); // We're now within
                                      // the function definition

        if (is_exact(L"(")) {
          lex(); // Consume the opening bracket

          while (!is_exact(L")")) {
            if (auto name = as<Token::ID>()) {
              auto decl = make_shared<SAST::FunctionArgumentDeclaration>();

              decl->name = name;
              prototype->args.insert(decl);

              lex(); // Consume the name

              if (is_exact(L","))
                lex(); // Consume the comma
            } else
              throw Error(_token, L"Expected argument name");
          }

          lex(); // Consume the closing bracket
        }

        skip_newlines();

        while (!is_end()) {
          // TODO: Parse expression
          lex();
        }

        lex();             // Consume the "end" keyword
        _sast_stack.pop(); // Exit the function definition
      } else {
        throw Error(_token, L"Unexpected token");
      }
    } else if (is<Token::Newline>()) {
      lex();
      continue;
    } else {
      throw Error(_token, L"Unexpected token");
    }
  }

  return nullopt;
}

void Parser::lex() {
  _token = _lexer->lex();
  debug_token();
};

void Parser::debug_token() {
  if (auto id = as<Token::ID>())
    wcerr << "ID\t" << id->value << "\n";
  else if (is<Token::Newline>())
    wcerr << "NEWLINE\t\n";
  else if (auto string = as<Token::StringLiteral>())
    wcerr << "STRING\t" << string->value << "\n";
  else if (auto other = as<Token::Other>())
    wcerr << "OTHER\t" << other->value << "\n";
  else if (is<Token::Eof>())
    wcerr << "EOF\n";
  else
    wcerr << "UNKNOWN\t\n";
}

template <class T> shared_ptr<T> Parser::as() {
  return dynamic_pointer_cast<T>(_token);
}

template <class T> bool Parser::is() { return (!!(as<T>())); }

bool Parser::is_exact(wstring value) {
  if (auto tok = as<Token::Other>())
    return tok->value == value;
  else
    return false;
}

bool Parser::is_end() {
  if (auto id = as<Token::ID>())
    return id->value == L"end";
  else
    return false;
}

void Parser::skip_newlines() {
  while (is<Token::Newline>())
    lex();
}

bool Parser::is_terminator() {
  return is<Token::Newline>() || is_exact(L";") || is<Token::Eof>();
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
//   bool is_terminator() { return is<Token::Newline>() || is<Token::Semi>() ||
//   is<Token::Eof>(); }
// };
} // namespace Onyx
