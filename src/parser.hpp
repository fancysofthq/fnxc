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
  shared_ptr<Token::Base> _token;
  stack<shared_ptr<SAST::Node>> _sast_stack;
  mutex *_sast_mutex;

public:
  struct Error {
    shared_ptr<Token::Base> token;
    const wstring reason;

    Error(shared_ptr<Token::Base> token, const wstring reason);
  };

  struct CompilationRequest {
    bool is_import;
    vector<fs::path> paths;

    CompilationRequest(bool is_import);
  };

  Parser(Lexer *lexer, shared_ptr<SAST::Node> sast_root, mutex *sast_mutex);

  optional<CompilationRequest> parse();
  shared_ptr<SAST::Expression> parse_expression();

private:
  void lex();
  void debug_token();

  template <class T> shared_ptr<T> as();
  template <class T> bool is();
  bool is_exact(wstring value);
  bool is_end();
  void skip_newlines();

  // Expressions are terminated with either newlines, semicolon or EOF.
  bool is_terminator();
};
} // namespace Onyx
