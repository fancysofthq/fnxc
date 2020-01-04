#include "./location.hpp"

Location::Location(
    fs::path path,
    uint32_t begin_row,
    uint32_t begin_column,
    uint32_t end_row,
    uint32_t end_column) :
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
