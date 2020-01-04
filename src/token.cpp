#include "./token.hpp"

namespace Onyx {
namespace Token {
wstring Control::pretty_source() {
  switch (kind) {
  case Eof:
    return L"<eof>";
  case Newline:
    return L"<newline>";
  case Space:
    return L"<space>";
  default:
    return source();
  }
}

wstring Control::source() {
  switch (kind) {
  case Eof:
    return 0;
  case Newline:
    return L"\n";
  case Space:
    return L" ";
  case Dot:
    return L".";
  case Semi:
    return L";";
  case Colon:
    return L":";
  case Comma:
    return L",";
  case Assignment:
    return L"=";
  case OpenParen:
    return L"(";
  case CloseParen:
    return L")";
  case OpenCurly:
    return L"{";
  case CloseCurly:
    return L"}";
  case OpenSquare:
    return L"[";
  case CloseSquare:
    return L"]";
  case Block:
    return L"~>";
  case BlockParen:
    return L"|";
  case Pipe:
    return L"|>";
  case KeyValue:
    return L"=>";
  }
}

wstring Value::pretty_kind() {
  switch (kind) {
  case Var:
    return L"VAR";
  case Type:
    return L"TYPE";
  case Op:
    return L"OP";
  case Intrinsic:
    return L"INTRNSC";
  case C:
    return L"C";
  case String:
    return L"STRING";
  case Comment:
    return L"COMMENT";
  case Macro:
    return L"MACRO";
  }
}

wstring AnonArg::source() { return L"&" + to_wstring(index); }

wstring DecimalInt::source() {
  wstring ret;

  for (auto d : digits)
    ret += d;

  if (type) {
    if (type == Signed)
      ret += 'i';
    else
      ret += 'u';

    if (bitsize)
      ret += to_wstring(bitsize);
  }

  return ret;
}

wstring DecimalFloat::source() {
  wstring ret;

  for (auto d : significand)
    ret += d;

  if (exponent != 0) {
    ret += 'e';
    ret += to_wstring(exponent);
  }

  switch (bitsize) {
  case Float16:
    ret += L"f16";
    break;
  case Float32:
    ret += L"f32";
    break;
  case Float64:
    ret += L"f64";
    break;
  case UndefBitsize:
    break;
  }

  return ret;
}

wstring NonDecimalNumber::source() {
  wstring ret;

  switch (kind) {
  case Bina:
    ret += L"0b";
    break;
  case Octa:
    ret += L"0o";
    break;
  case Hexa:
    ret += L"0x";
    break;
  }

  for (auto c : chars)
    ret += c;

  switch (type) {
  case SignedInt:
    ret += 'i';
    break;
  case UnsignedInt:
    ret += 'u';
    break;
  case Float:
    ret += 'f';
    break;
  case UndefType:
    break;
  }

  if (bitsize)
    ret += to_wstring(bitsize);

  return ret;
}
} // namespace Token
} // namespace Onyx
