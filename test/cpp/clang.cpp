#include <iostream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>

#include "clang-c/CXErrorCode.h"
#include "clang-c/CXString.h"
#include "clang-c/Index.h"

/*
  1. Remove annotations
  2. Remove function bodies
  3. Remove Onyx macros
*/

void diagnose(CXTranslationUnit);
void print_token(CXTranslationUnit, CXToken);
void print_tokens(CXTranslationUnit, CXCursor);

CXChildVisitResult
visitor(CXCursor cursor, CXCursor parent, CXClientData);

int main() {
  CXIndex index = clang_createIndex(0, 0);

  CXTranslationUnit unit;

  int flags =
      CXTranslationUnit_Incomplete |
      CXTranslationUnit_SkipFunctionBodies |
      CXTranslationUnit_IncludeBriefCommentsInCodeCompletion |
      CXTranslationUnit_KeepGoing |
      CXTranslationUnit_SingleFileParse |
      CXTranslationUnit_IncludeAttributedTypes;

  auto parse_error = clang_parseTranslationUnit2(
      index,
      "D:/Dev/fancysoft/nxc/test/cpp/temp.h",
      nullptr,
      0,
      nullptr,
      0,
      flags,
      &unit);

  if (parse_error) {
    throw std::domain_error(
        "Unit parse error: " + std::to_string(parse_error));
  }

  diagnose(unit);

  auto cursor = clang_getTranslationUnitCursor(unit);
  print_tokens(unit, cursor);

  auto _ = clang_visitChildren(cursor, *visitor, 0);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);

  std::cout << "Done" << std::endl;
}

void print_token(CXTranslationUnit unit, CXToken token) {
  auto string = clang_getTokenSpelling(unit, token);
  auto kind = clang_getTokenKind(token);

  switch (kind) {
  case CXToken_Comment:
    std::cout << "COMMENT\t" << clang_getCString(string)
              << std::endl;
    break;
  case CXToken_Identifier:
    std::cout << "ID\t" << clang_getCString(string) << std::endl;
    break;
  case CXToken_Keyword:
    std::cout << "KEYWORD\t" << clang_getCString(string)
              << std::endl;
    break;
  case CXToken_Literal:
    std::cout << "LITERAL\t" << clang_getCString(string)
              << std::endl;
    break;
  case CXToken_Punctuation:
    std::cout << "PUNCT\t" << clang_getCString(string) << std::endl;
    break;
  default:
    throw std::domain_error(
        "Unhandled token kind " + std::to_string(kind));
  }
}

void print_tokens(CXTranslationUnit unit, CXCursor cursor) {
  CXToken *tokens;
  unsigned int tokens_count;
  auto src_range = clang_getCursorExtent(cursor);

  clang_tokenize(unit, src_range, &tokens, &tokens_count);

  for (int i = 0; i < tokens_count; i++) {
    auto token = tokens[i];
    print_token(unit, token);
  }

  clang_disposeTokens(unit, tokens, tokens_count);
}

CXChildVisitResult function_decl_visitor(
    CXCursor cursor, CXCursor parent, CXClientData data) {
  auto kind = clang_getCursorKind(cursor);
  auto type = clang_getCursorType(cursor);

  if (kind == CXCursor_ParmDecl) {
    auto kind_name = clang_getCursorSpelling(cursor);
    auto type_name = clang_getTypeSpelling(type);

    std::cout << "ParmDecl `" << clang_getCString(kind_name)
              << "` of type `" << clang_getCString(type_name) << "`"
              << std::endl;

    int *argc = (int *)data;
    (*argc)++;
  }

  return CXChildVisit_Continue;
}

CXChildVisitResult var_decl_visitor(
    CXCursor cursor, CXCursor parent, CXClientData data) {
  auto kind = clang_getCursorKind(cursor);
  auto type = clang_getCursorType(cursor);

  switch (kind) {
  case CXCursor_StringLiteral: {
    auto res = clang_Cursor_Evaluate(cursor);
    auto value = clang_EvalResult_getAsStr(res);
    clang_EvalResult_dispose(res);

    std::cout << clang_getCString(clang_getCursorKindSpelling(kind))
              << " " << value << std::endl;

    break;
  }

  case CXCursor_IntegerLiteral: {
    auto res = clang_Cursor_Evaluate(cursor);
    auto value = clang_EvalResult_getAsInt(res);
    clang_EvalResult_dispose(res);

    std::cout << clang_getCString(clang_getCursorKindSpelling(kind))
              << " " << value << std::endl;

    break;
  }

  default:
    break;
  }

  return CXChildVisit_Recurse;
}

CXChildVisitResult
visitor(CXCursor cursor, CXCursor parent, CXClientData data) {
  auto kind = clang_getCursorKind(cursor);
  auto name = clang_getCursorSpelling(cursor);

  switch (kind) {
  case CXCursor_FunctionDecl: {
    std::cout << "FunctionDecl `" << clang_getCString(name) << "`"
              << std::endl;

    int argc = 0;
    clang_visitChildren(cursor, *function_decl_visitor, &argc);

    std::cout << "Params count: " << argc << std::endl;

    return CXChildVisit_Continue;
  }
  case CXCursor_VarDecl: {
    auto type = clang_getCursorType(cursor);

    std::cout << "VarDecl `" << clang_getCString(name)
              << "` of type `"
              << clang_getCString(clang_getTypeSpelling(type)) << "`"
              << std::endl;

    clang_visitChildren(cursor, *var_decl_visitor, nullptr);

    return CXChildVisit_Continue;
  }
  default:
    return CXChildVisit_Recurse;
  }
}

void diagnose(CXTranslationUnit unit) {
  const int count = clang_getNumDiagnostics(unit);
  std::cout << "Diagnostics count: " << count << std::endl;

  for (int i = 0; i < count; i++) {
    auto diag = clang_getDiagnostic(unit, i);
    auto string = clang_formatDiagnostic(
        diag, clang_defaultDiagnosticDisplayOptions());
    std::cout << clang_getCString(string) << std::endl;
    clang_disposeString(string);
  }
}
