#include <memory>
#include <mutex>
#include <optional>
#include <stack>

#include "./ast.hpp"
#include "./lexer.hpp"

using namespace std;
namespace fs = experimental::filesystem;

namespace Onyx {
namespace Compiler {

// Parses the tokens into the abstract syntax tree (AST).
// The tree may be written into by multiple parsers
// simulataneously, therefore it must be synchronized.
class Parser {
  Lexer *_lexer;

  // The latest lexed token.
  shared_ptr<Token::Base> _token;

  // This file's AST root.
  shared_ptr<AST::Node> _AST_root;

public:
  struct Error {
    shared_ptr<Token::Base> token;
    const string reason;

    Error(shared_ptr<Token::Base> token, const string reason);
  };

  struct Require {
    // The keyword token.
    const shared_ptr<Token::Value> token;

    const bool is_import;
    const fs::path path;

    Require(
        const shared_ptr<Token::Value> token,
        const bool is_import,
        const fs::path);
  };

  Parser(Lexer *, shared_ptr<AST::Node> root);

  // Parse the file's requires (including imports).
  // By the language standards, requires can only
  // be in the very top of a file.
  stack<Require> requirements();

  // Continue parsing the file.
  shared_ptr<AST::Node> next();

  // shared_ptr<AST::Expression> parse_expression();

private:
  void _lex();
  void debug_token();

  template <class T> shared_ptr<T> as();
  template <class T> bool is();

  bool is_exact(wstring value);
  bool is_end();
  bool is_newline();
  bool is_eof();
  void skip_newlines();

  // An expression is terminated with either a newline, semicolon or
  // EOF.
  bool is_terminator();
};
} // namespace Compiler
} // namespace Onyx
