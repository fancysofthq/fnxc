require("include/table")
require("include/utf8")

local describe = require("include/describe")
local tokenizer = require("src/tokenizer")

local function pseudotab(string)
    local buffer = ""

    for i = 0, 7 - string:len() do buffer = buffer .. " " end

    return buffer
end

local function pretty_tokens(input)
    assert(type(input) == "string")

    local file = io.tmpfile()
    file:write(input)
    file:seek("set", 0)

    local coro = tokenizer.tokenize(file)
    local buffer = ""

    while true do
        local _, token = coroutine.resume(coro)

        if not token then break end

        if not _ then error(token) end

        if token.error then
            error(token.error .. " at " .. token.start_row .. ":" ..
                      token.start_col)
        end

        if token.value == nil or token.value == '' then
            buffer = buffer .. token.type .. "\n"
        else
            local value = token.value

            if token.type == "TEXT" then
                value = value:gsub(" ", "·")
                value = value:gsub("\n", "\\n")
            end

            buffer = buffer .. token.type .. pseudotab(token.type) .. value ..
                         "\n"
        end
    end

    return buffer
end

local function assert_tokens(input, expected)
    local expected_lines = utf8.split(expected, "\n")
    local output = pretty_tokens(input)

    local row = 0

    for output_line in table.values(utf8.split(output, "\n")) do
        row = row + 1
        local expected_line = expected_lines[row]
        assert(output_line == expected_line,
               string.format("Expected `%s`, got `%s` at line %d",
                             expected_line, output_line, row))
    end
end

describe("Tokenizer", function(it)
    it("handles simple expressions", function()
        assert_tokens([[
foo = !(bar)  ** 22
]], [[
ID      foo
OP      =
OP      !
BRACKET (
ID      bar
BRACKET )
OP      **
NUMERIC 22
EOF
]])
    end)

    it("handles calls", function()
        assert_tokens([[
foo.bar(42)
]], [[
ID      foo
CONTROL .
ID      bar
BRACKET (
NUMERIC 42
BRACKET )
EOF
]])
    end)

    it("handles calls with do blocks", function()
        assert_tokens([[
foo do |x, y|
  x + y
end
]], [[
ID      foo
KEYWORD do
BLCKARG |
ID      x
CONTROL ,
ID      y
BLCKARG |
CONTROL \n
ID      x
OP      +
ID      y
CONTROL \n
KEYWORD end
EOF
]])
    end)

    it("handles calls with {} blocks", function()
        assert_tokens([[
bar{ |x| puts(x) }
]], [[
ID      bar
BRACKET {
BLCKARG |
ID      x
BLCKARG |
ID      puts
BRACKET (
ID      x
BRACKET )
BRACKET }
EOF
]])
    end)

    it("handles multilines", function()
        assert_tokens([[
foo = 1  +
    {bar   **  baz}

  qux =   22

]], [[
ID      foo
OP      =
NUMERIC 1
OP      +
CONTROL \n
BRACKET {
ID      bar
OP      **
ID      baz
BRACKET }
CONTROL \n
CONTROL \n
ID      qux
OP      =
NUMERIC 22
CONTROL \n
EOF
]])
    end)

    it("handles quoted strings", function()
        assert_tokens([[
foo = "bar baz\nqux"
]], [[
ID      foo
OP      =
QUOTE   "
TEXT    bar·baz\nqux
QUOTE   "
EOF
]])
    end)

    it("handles interpolation", function()
        assert_tokens([[
"foo #{bar + " baz #{42}"} qux "
]], [[
QUOTE   "
TEXT    foo·
CONTROL #
BRACKET {
ID      bar
OP      +
QUOTE   "
TEXT    ·baz·
CONTROL #
BRACKET {
NUMERIC 42
BRACKET }
TEXT
QUOTE   "
BRACKET }
TEXT    ·qux·
QUOTE   "
EOF
]])
    end)

    it("handles percentage string literals", function()
        assert_tokens([[
%q("bar baz#{42}\nqux")
]], [[
PERCENT %q
BRACKET (
TEXT    "bar·baz#{42}\nqux"
BRACKET )
EOF
]])
    end)

    it("handles heredocs", function()
        assert_tokens([[
foo << %sql[
  SELECT
A SQL
    B SQL (nah, joking)
    ] + bar
]], [[
ID      foo
OP      <<
PERCENT %sql
BRACKET [
CONTROL \n
TEXT    ··SELECT\nA·SQL\n····B·SQL·(nah,·joking)
CONTROL \n
BRACKET ]
OP      +
ID      bar
EOF
]])
    end)

    it("handles heredocs with interpolation", function()
        assert_tokens([[
%SQL(
  SELECT (foo\)
    #{(bar+
  42) + %sql{
  Whaa,  \nanother
heredoc here?
  }
}
      SOMETHING
)
]], [[
PERCENT %SQL
BRACKET (
CONTROL \n
TEXT    ··SELECT·(foo)\n····
CONTROL #
BRACKET {
BRACKET (
ID      bar
OP      +
CONTROL \n
NUMERIC 42
BRACKET )
OP      +
PERCENT %sql
BRACKET {
CONTROL \n
TEXT    ··Whaa,··\nanother\nheredoc·here?
CONTROL \n
BRACKET }
CONTROL \n
BRACKET }
TEXT    \n······SOMETHING
CONTROL \n
BRACKET )
EOF
]])
    end)

    it("handles object declarations", function()
        assert_tokens([[
class Foo(T)
  derive Bar(T)

  struct Baz(Size N : IntLiteral, U) < Qux(V: U)
  end
end
]], [[
KEYWORD class
ID      Foo
BRACKET (
ID      T
BRACKET )
CONTROL \n
KEYWORD derive
ID      Bar
BRACKET (
ID      T
BRACKET )
CONTROL \n
CONTROL \n
KEYWORD struct
ID      Baz
BRACKET (
ID      Size
ID      N
CONTROL :
ID      IntLiteral
CONTROL ,
ID      U
BRACKET )
OP      <
ID      Qux
BRACKET (
NARG    V
ID      U
BRACKET )
CONTROL \n
KEYWORD end
CONTROL \n
KEYWORD end
EOF
]])
    end)

    it("handles function declarations", function()
        assert_tokens([[
private def foo(bar, baz qux : Integer(32) | String = "foo bar",
  ..vargs : Glyph(T)) forall T : U
    return a +
      b
  end
]], [[
KEYWORD private
KEYWORD def
ID      foo
BRACKET (
ID      bar
CONTROL ,
ID      baz
ID      qux
CONTROL :
ID      Integer
BRACKET (
NUMERIC 32
BRACKET )
OP      |
ID      String
OP      =
QUOTE   "
TEXT    foo·bar
QUOTE   "
CONTROL ,
CONTROL \n
CONTROL ..
ID      vargs
CONTROL :
ID      Glyph
BRACKET (
ID      T
BRACKET )
BRACKET )
KEYWORD forall
ID      T
CONTROL :
ID      U
CONTROL \n
KEYWORD return
ID      a
OP      +
CONTROL \n
ID      b
CONTROL \n
KEYWORD end
EOF
]])
    end)

    it("handles procs (#1)", function()
        assert_tokens([[
proc = (Int32, Int32) ~> (Int32) do |a, b|
  return a + b
end
]], [[
ID      proc
OP      =
BRACKET (
ID      Int32
CONTROL ,
ID      Int32
BRACKET )
CONTROL ~>
BRACKET (
ID      Int32
BRACKET )
KEYWORD do
BLCKARG |
ID      a
CONTROL ,
ID      b
BLCKARG |
CONTROL \n
KEYWORD return
ID      a
OP      +
ID      b
CONTROL \n
KEYWORD end
EOF
]])
    end)

    it("handles procs (#2)", function()
        assert_tokens([[
proc = Int32 ~> { puts(& + &2) }
]], [[
ID      proc
OP      =
ID      Int32
CONTROL ~>
BRACKET {
ID      puts
BRACKET (
OP      &
OP      +
ANONARG &2
BRACKET )
BRACKET }
EOF
]])
    end)
end)
