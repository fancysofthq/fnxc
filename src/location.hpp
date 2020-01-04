#pragma once

#include <experimental/filesystem>

using namespace std;
namespace fs = experimental::filesystem;

struct Location {
  fs::path path;

  uint32_t begin_row;
  uint32_t begin_column;

  uint32_t end_row;
  uint32_t end_column;

  Location(
      fs::path path,
      uint32_t begin_row,
      uint32_t begin_column,
      uint32_t end_row,
      uint32_t end_column);

  Location(fs::path path, Location loc);
  Location(fs::path path);
  Location();
};
