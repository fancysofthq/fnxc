#include <memory>

#include "../../../include/fnx/cnx/parser.hpp"
#include "../../../include/fnx/errors.hpp"
#include "../../../include/fnx/lua/parser.hpp"
#include "../../../include/fnx/onyx/cst.hpp"
#include "../../../include/fnx/onyx/parser.hpp"

namespace FNX {
namespace Onyx {

Parser::Parser(std::shared_ptr<File> unit) : _unit(unit) {}

/// A shortcut to dynamically cast a variant instance.
#define _CAST(TYPE, VARIANT) std::get_if<Token::TYPE>(VARIANT)

/// A shortcut to check if a result of `_CAST` is a keyword token.
#define _IS_KW(PVAL, KIND) PVAL->kind == Token::Keyword::KIND

/// A shortcut to check if a result of `_CAST` is a punctuation
/// token.
#define _IS_PUNCT(PVAL, KIND) PVAL->kind == Token::Punctuation::KIND

void Parser::parse() {
  Lexer lexer(_unit->source_stream());

  auto coro = lexer.lex();
  while (!coro.done()) {
    auto token = coro.next();

    auto token_ptr = std::make_shared<File::AnyToken>(token);
    _unit->tokens.push_back(token_ptr);

    if (auto ptr = _CAST(Token::Keyword, &token)) {
      // Handle an `export` statement.
      if (_IS_KW(ptr, Export)) {
        if (coro.done())
          throw UnexpectedEOF();

        auto next = coro.next();
        auto ptr = _CAST(Punctuation, &next);

        if (!ptr ||
            !(_IS_PUNCT(ptr, Space) || _IS_PUNCT(ptr, CurlyOpen)))
          throw Error("Expected space or '{'");

        // Roll back to the position right after `export`.
        lexer.retreat();

        auto block = make_shared<CNX::Block>(_unit->source_stream());
        CNX::Parser cnx_parser(block);

        cnx_parser.parse();
      }
    }

    else if (auto ptr = _CAST(Punctuation, &token)) {
      switch (ptr->kind) {
      case Token::Punctuation::C: {
        break;
      }

      case Token::Punctuation::MacroOpen:
      case Token::Punctuation::EmittingMacroOpen:
      case Token::Punctuation::DelayedMacroOpen:
      case Token::Punctuation::DelayedEmittingMacroOpen:
      case Token::Punctuation::FinalMacroOpen:
      case Token::Punctuation::FinalEmittingMacroOpen: {
        CST::Macro::Kind kind;
        bool is_emitting;

        switch (ptr->kind) {
        case Token::Punctuation::EmittingMacroOpen:
          is_emitting = true;
        case Token::Punctuation::MacroOpen:
          kind = CST::Macro::Immediate;
          break;
        case Token::Punctuation::DelayedEmittingMacroOpen:
          is_emitting = true;
        case Token::Punctuation::DelayedMacroOpen:
          kind = CST::Macro::Delayed;
          break;
        case Token::Punctuation::FinalEmittingMacroOpen:
          is_emitting = true;
        case Token::Punctuation::FinalMacroOpen:
          kind = CST::Macro::Final;
          break;
        default:
          throw ICE(); // Unreacheable
        }

        auto block = std::make_shared<Lua::Block>(_unit);
        auto lua_parser = Lua::Parser(block);

        // Roll back to the position right after
        // the macro opening sequence.
        lexer.retreat();

        auto terminator = is_emitting
                              ? Lua::Lexer::Terminator::Emitting
                              : Lua::Lexer::Terminator::NonEmitting;

        lua_parser.parse(terminator);

        auto next = coro.next();
        auto ptr = _CAST(next, Token::Punctuation);

        CST::Macro node(
            kind,
            is_emitting,
            block,
            std::dynamic_pointer_cast<Token::Punctuation>(token_ptr),
            nullptr);

        break;
      }

      default:; // TODO:
      }
    }
  }
}

} // namespace Onyx
} // namespace FNX
