#include <fstream>
#include <ios>
#include <istream>
#include <sstream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "fnx-test/protobuf/foo.pb.h"

TEST_CASE("Basic") {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  FNX::Protobuf::Foo write_msg;

  write_msg.set_bar("Hello world!");
  write_msg.set_baz(42);

  std::stringstream buffer;
  REQUIRE(write_msg.SerializeToOstream(&buffer));

  FNX::Protobuf::Foo read_msg;
  REQUIRE(read_msg.ParseFromIstream(&buffer));

  CHECK(read_msg.bar().compare("Hello world!") == 0);
  CHECK(read_msg.baz() == 42);
}
