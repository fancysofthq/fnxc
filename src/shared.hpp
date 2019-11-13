#pragma once

#include <experimental/filesystem>

using namespace std;
namespace fs = experimental::filesystem;

wstring to_wstring(const char *bytes);

struct Location {
  fs::path path;

  uint begin_row;
  uint begin_column;

  uint end_row;
  uint end_column;

  Location(
      fs::path path,
      uint begin_row,
      uint begin_column,
      uint end_row,
      uint end_column);

  Location(fs::path path, Location loc);
  Location(fs::path path);
  Location();
};
