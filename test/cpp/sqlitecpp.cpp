#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "SQLiteCpp/Database.h"
#include "sqlite3.h"

TEST_CASE("Basic") {
  SQLite::Database db(
      "test", SQLite::OPEN_READWRITE | SQLITE_OPEN_MEMORY);
  SQLite::Statement query(db, "SELECT 42");
  query.executeStep();
  int x = query.getColumn(0);
  CHECK(x == 42);
}
