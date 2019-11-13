#include "./shared.hpp"

// Code taken from https://stackoverflow.com/a/51077426/3645337.
wstring to_wstring(const char *bytes) {
  using convert_type = codecvt_utf8<typename wstring::value_type>;
  wstring_convert<convert_type, typename wstring::value_type> converter;
  return converter.from_bytes(bytes);
}

Location::Location(
    fs::path path,
    uint begin_row,
    uint begin_column,
    uint end_row,
    uint end_column) :
    path(path),
    begin_row(begin_row),
    begin_column(begin_column),
    end_row(end_row),
    end_column(end_column) {}

Location::Location(fs::path path, Location loc) : path(path) {
  begin_row = loc.begin_row;
  begin_column = loc.begin_column;
  end_row = loc.end_row;
  end_column = loc.end_column;
}

Location::Location(fs::path path) : path(path) {}

Location::Location() {}
