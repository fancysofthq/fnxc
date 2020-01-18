#include <memory>
#include <mutex>
#include <optional>
#include <stack>

#include "./lexer.hpp"
#include "./sast.hpp"

using namespace std;
namespace fs = experimental::filesystem;

namespace Onyx {
class Parser {
  Lexer *_lexer;

  // The latest lexed token.
  shared_ptr<Token::Base> _token;

  // This file SAST root.
  shared_ptr<SAST::Node> _sast_root;

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

  Parser(Lexer *, shared_ptr<SAST::Node> root);

  // Parse the file's requires (including imports).
  // By the language standards, requires can only
  // be in the very top of a file.
  stack<Require> requirements();

  // Continue parsing the file.
  shared_ptr<SAST::Node> next();

  // shared_ptr<SAST::Expression> parse_expression();

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

  // Expressions are terminated with either newlines, semicolon or EOF.
  bool is_terminator();
};
} // namespace Onyx
