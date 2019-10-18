local tokenizer = {}

require("include/table")
require("include/utf8")

local reader = require("src/reader")

-- Keywords used in Onyx
local keywords = {
  ["require"] = true,
  ["do"] = true,
  ["end"] = true,
  namespace = true,
  module = true,
  primitive = true,
  struct = true,
  class = true,
  enum = true,
  flag = true,
  derive = true,
  protected = true,
  private = true,
  abstract = true,
  def = true,
  ["self"] = true,
  forall = true,
  ["return"] = true,
  ["if"] = true,
  ["then"] = true,
  unless = true,
  ["else"] = true,
  ["while"] = true,
  ["until"] = true,
  next = true,
  ["break"] = true,
  case = true,
  when = true,
  ["yield"] = true
}

local percent_modifiers = {
  q = false, -- q — A quoted Twine, probably including the `'` glyph
  Q = true, -- Q — Interpolation-enabled String, prob. incl. `"`
  w = false, -- w — Words (slice of space-separated Twines)
  W = true, -- W — Interpolation-enabled String words
  i = false, -- i — Slice of Symbols
  g = false -- g — Slice of Glyphs, probably including the ``` itself
}

local function reverse_bracket(codepoint)
  if codepoint == 40 then -- (
    return 41 -- )
  elseif codepoint == 91 then -- [
    return 93 -- ]
  elseif codepoint == 123 then -- {
    return 125 -- }
  else
    error("Unknown bracket " .. utf8.char(codepoint))
  end
end

-- The tokenizer itself.
function tokenizer.tokenize(input)
  -- The current codepoint
  local codepoint = nil

  -- End Of File
  local eof = false

  -- Current cursor position
  local row = 1
  local col = 1

  -- Starting token position
  local srow = 1
  local scol = 1

  -- Set starting position of the next token to current cursor.
  local function reset_pos()
    srow = row
    scol = col
  end

  -- Set to `true` when we expect an identificator
  local id_required = false

  -- Is it currently varg?
  local varg = false

  -- Are block args currently expected?
  local block = false

  -- Are we currently within block args declaration?
  local within_block_args = false

  -- The codepoints buffer
  local buffer = {}

  -- Push current codepoint into the codepoints buffer.
  local function buff()
    table.insert(buffer, codepoint)
  end

  -- The function which yields the next token.
  local function yield(token_type, value)
    if type(value) == "table" then
      value = utf8.chars(value)
    elseif type(value) == "number" then
      value = utf8.char(value)
    end

    -- print(token_type .. "\t" .. tostring(value))

    coroutine.yield {
      start_row = srow,
      start_col = scol,
      end_row = row,
      end_col = col,
      type = token_type,
      value = value
    }

    reset_pos()
    buffer = {}
  end

  local function yield_error(message)
    coroutine.yield {
      start_row = srow,
      start_col = scol,
      end_row = row,
      end_col = col,
      error = message
    }
  end

  -- Expression terminators, i.e. brackets and quotes
  local terminators = {}

  -- The coroutine to read the input file
  local read_coro = reader.read(input)

  -- A function which returns the next codepoint read.
  local function read()
    local _, res = coroutine.resume(read_coro)

    if res then
      row = res.row
      col = res.col
      codepoint = res.codepoint
    else
      codepoint = -1
    end
  end

  -- A convenient function to buff() and then read().
  local function buffread()
    buff()
    read()
  end

  -- Saves at least 8 characters to type!
  local function is(right, left)
    if left == nil then
      left = codepoint
    end

    if left == -1 then
      return false
    else
      return utf8.char(left) == right
    end
  end

  -- Is a letter from latin alphabet?
  local function is_latin(c)
    if c == nil then
      c = codepoint
    end

    return (c >= 65 and c <= 90) or (c >= 97 and c <= 122) -- a-z; A-Z
  end

  -- Is a letter from greek alphabet?
  local function is_greek(c)
    if c == nil then
      c = codepoint
    end

    return (c >= 913 and c <= 929) or (c >= 931 and c <= 937) or
      (c >= 945 and c <= 969) -- Α-Ρ; Σ-Ω; α-ω
  end

  -- Is a letter from either latin or greek alphabet?
  local function is_alpha(c)
    return is_latin(c) or is_greek(c)
  end

  -- Is a digit?
  local function is_digit(c)
    if c == nil then
      c = codepoint
    end

    return c >= 48 and c <= 57
  end

  local function is_whitespace(c)
    return (c or codepoint) == 32
  end

  local function is_newline(c)
    return (c or codepoint) == 10
  end

  local function is_dot(c)
    return (c or codepoint) == 46
  end

  local function is_underscore(c)
    return (c or codepoint) == 95
  end

  -- Is one of `([{`?
  local function is_opening_bracket(c)
    if c == nil then
      c = codepoint
    end

    return c == 40 or c == 91 or c == 123
  end

  -- Is one of `)]}`?
  local function is_closing_bracket(c)
    if c == nil then
      c = codepoint
    end

    return c == 41 or c == 93 or c == 125
  end

  -- Is one of `"'` or ` itself?
  local function is_quote(c)
    if c == nil then
      c = codepoint
    end

    return c == 34 or c == 39 or c == 96
  end

  -- Is one of `,:;?`?
  local function is_control(c)
    if c == nil then
      c = codepoint
    end

    return c == 44 or c == 58 or c == 59 or c == 63
  end

  -- Is `.` or `@`?
  local function is_access(c)
    if c == nil then
      c = codepoint
    end

    return c == 46 or c == 64
  end

  local function is_op(c)
    return not (is_alpha(c) or is_digit(c) or is_opening_bracket(c) or
      is_closing_bracket(c) or
      is_quote(c) or
      is_control(c) or
      is_whitespace(c) or
      is_newline(c))
  end

  -- Assume the next block of chars to be a text literal,
  -- which terminates upon hitting the *terminator*; and read it.
  --
  -- If *enable_interpolation* is set to `false`, `#{` is ignored.
  --
  -- The function returns `true` if the terminator is met (which
  -- may be false if it ended with interpolation).
  --
  -- Codepoint is set to either the interpolation bracket (`{`),
  -- or to the terminator met. Note that terminator itself is not yielded yet.
  local function read_text(terminator, enable_interpolation)
    local backslash = false
    local terminated = nil
    local empty_line = true

    while true do
      if is("\n") then
        if backslash then
          -- Do not put the newline into the buffer
        else
          empty_line = true
          buff()
        end

        read()
      elseif enable_interpolation and (is("#") and not backslash) then
        read() -- Consume the `#`

        if is("{") then
          terminated = false

          yield("TEXT", buffer)
          yield("CONTROL", "#")

          table.insert(
            terminators,
            {codepoint = 125, interpolation = true}
          ) -- }
          yield("BRACKET", "{")

          read()
          break
        else
          -- False alarm
          table.insert(buffer, "#")
          buff()
        end
      elseif is("\\") then
        backslash = true
        read()
      elseif codepoint == terminator and not backslash then
        local newline = false

        if empty_line then
          -- Remove whitespace
          while buffer[#buffer] == 32 do
            table.remove(buffer)
          end

          -- Remove the newline
          if buffer[#buffer] == 10 then
            table.remove(buffer)
            newline = true
          end
        end

        yield("TEXT", buffer)
        terminated = true

        if newline then
          yield("CONTROL", "\\n")
        end

        break
      else
        if backslash then
          -- \n
          if codepoint == 110 then
            table.insert(buffer, 92) -- \
          end

          backslash = false
        end

        -- Whitespaces keep the line empty
        if not (codepoint == 32) then
          empty_line = false
        end

        buffread()
      end
    end

    return terminated
  end

  local function next()
    -- TODO: Call modifiers (?)

    if is_alpha() or is_underscore() then
      repeat
        buffread()
      until not (is_alpha() or is_digit() or is_underscore())

      if is(":") then
        yield("NARG", buffer)
        read()
      else
        -- Append trailing `!` or `?` or `*`
        if is("!") or is("?") or is("*") then
          buffread()
        end

        local content = utf8.chars(buffer)

        if keywords[content] then
          yield("KEYWORD", content)

          if content == "do" then
            block = true
          elseif content == "end" then
            block = false
          end
        else
          yield("ID", content)
          id_required = false
        end
      end
    elseif id_required then
      yield_error("Identificator expected")
    elseif is_digit() then
      local ends_with_underscore = false
      local dot = false
      local ends_with_dot = false
      local first_digit_codepoint = codepoint

      buffread() -- Read the second char

      -- Returns `true` if current codepoint matches
      -- base requirements (e.g. 1-7 for octal numbers)
      local conditional = nil

      if is_digit() or is_dot() or is_underscore() then
        -- This conditional is applicable to decimal numbers only.
        conditional = function()
          return is_digit()
        end
      elseif is("0", first_digit_codepoint) then
        if is("b") then
          -- Binary numbers consist of 1s and 0s only
          conditional = function()
            return codepoint == 48 or codepoint == 49
          end
        elseif is("o") then
          -- Octal numbers consist of digits from 0 to 7
          conditional = function()
            return codepoint >= 48 and codepoint <= 55
          end
        elseif is("x") then
          -- Hexadecimal numbers consist of digits and letters a-f and A-F
          conditional = function()
            local c = codepoint -- For convenience

            return is_digit() or (c >= 65 and c <= 70) or
              (c >= 97 and c <= 102)
          end
        else
          yield_error("Unknown numeric base")
        end
      end

      if conditional then
        repeat
          if is_underscore() then
            ends_with_underscore = true
          else
            ends_with_underscore = false
          end

          if is_dot() then
            if dot then
              yield_error("Number already has a dot")
            else
              dot = true
              ends_with_dot = true
            end
          else
            ends_with_dot = false
          end

          buffread()
        until not conditional() or is_dot() or is_underscore()
      end

      if ends_with_dot then
        -- Special case: if a number ends with a dot, it's a call then

        table.remove(buffer) -- Pop the dot from the number
        yield("NUMERIC", buffer)

        yield("CONTROL", ".")
        id_required = true
      else
        -- Floats can have explicit exponent
        if is("e") then
          buffread()

          -- The exponent can be negative
          if is("-") then
            buffread()
          end

          if conditional() then
            repeat
              buffread()
            until not conditional()
          else
            yield_error("Exponent value expected")
          end
        end

        -- Explicit literals
        if is("f") or is("i") or is("u") then
          buffread()

          if is_digit(codepoint) then
            -- Read the bitsize
            repeat
              buffread()
            until not is_digit(codepoint)
          end
        elseif ends_with_underscore then
          -- We're expecting the continuation
          yield_error("Numeric literals can not end with underscore")
        end

        yield("NUMERIC", buffer)
      end
    elseif is_whitespace(codepoint) then
      read() -- Consume the whitespace
      reset_pos() -- Reset the upcoming token starting position
    elseif is_newline() then
      read() -- Consume the newline
      yield("CONTROL", "\\n")
      block = false
    elseif is("#") then
      read() -- Consume the `#` char

      repeat
        buffread()
      until is_newline()

      yield("COMMENT", buffer)
    elseif is_opening_bracket() then
      if codepoint == 123 then -- {
        block = true
      end

      table.insert(
        terminators,
        {
          codepoint = reverse_bracket(codepoint),
          interpolation = false
        }
      )

      yield("BRACKET", codepoint)
      read()
    elseif is_closing_bracket() then
      local bracket = terminators[#terminators]

      if bracket and bracket.codepoint == codepoint then
        if codepoint == 125 then -- }
          block = false
        end

        table.remove(terminators)
        yield("BRACKET", codepoint)
        read() -- Consume the bracket

        if bracket.interpolation then
          local terminator = terminators[#terminators]

          if terminator then
            local terminated = read_text(terminator.codepoint, true)

            if terminated then
              table.remove(terminators)

              if is_quote() then
                yield("QUOTE", terminator.codepoint)
              else
                yield("BRACKET", terminator.codepoint)
              end

              read()
            end
          else
            error("BUG: There must be a terminator")
          end
        end
      else
        yield_error("Unmatched closing bracket")
      end
    elseif is_quote() then
      local quote = codepoint

      yield("QUOTE", quote)
      read()

      -- Interpolation is only enabled for double quotes
      local interpolation = quote == 34

      table.insert(
        terminators,
        {codepoint = quote, interpolation = interpolation}
      )

      local terminated = read_text(quote, interpolation)

      if terminated then
        table.remove(terminators)
        yield("QUOTE", quote)
        read()
      end
    elseif is(",") or is(":") or is(";") or is("?") then
      yield("CONTROL", codepoint)
      read()
    elseif is(".") or is("@") then
      local cp = codepoint
      read()

      if is(".") then
        yield("CONTROL", "..")
        read()
      else
        yield("CONTROL", cp)
      end

      id_required = true
    elseif is("~") then
      buffread()

      if is(">") then
        yield("CONTROL", "~>")
        read()
      else
        while is_op() do
          buffread()
        end

        yield("OP", buffer)
      end
    elseif is("%") then
      buffread() -- Consume the % char

      local modifier = {}

      if is_latin() then
        -- That IS a percent literal (one of places where spaces matter)

        repeat
          table.insert(modifier, codepoint)
          buffread()
        until not (is_latin() or is_underscore())
      end

      if is_opening_bracket() then
        local interpolation = true

        if percent_modifiers[utf8.chars(modifier)] == false then
          interpolation = false
        end

        if modifier then
          yield("PERCENT", buffer)
        else
          yield("PERCENT", buffer)
        end

        yield("BRACKET", codepoint)

        local terminator = reverse_bracket(codepoint)

        table.insert(
          terminators,
          {
            codepoint = terminator,
            interpolation = interpolation
          }
        )

        read()

        if is_newline() then
          yield("CONTROL", "\\n")
          read() -- Consume the newline
        end

        local terminated = read_text(terminator, interpolation)

        if terminated then
          table.remove(terminators)
          yield("BRACKET", terminator)
          read() -- Consume the terminator bracket
        end
      elseif modifier then
        yield_error("Opening bracket expected for percent literal")
      else
        -- That's an operation then (or maybe an annotation?)

        while is_op() do
          buffread()
        end

        yield("OP", buffer)
      end
    elseif is("&") then
      buffread()

      if is_alpha() or is_underscore() then
        yield("POINTER", buffer)
        id_required = true
      elseif is_digit() then
        while is_digit() do
          buffread()
        end

        yield("ANONARG", buffer)
      else
        while is_op() do
          buffread()
        end

        yield("OP", buffer)
      end
    elseif is("|") and block then
      buffread()
      yield("BLCKARG", buffer)

      if within_block_args then
        within_block_args = false
        block = false
      else
        within_block_args = true
      end
    elseif is_op() then
      while is_op() do
        buffread()
      end

      yield("OP", buffer)
    else
      error("BUG: Unhandled codepoint at " .. row .. ":" .. col)
    end
  end

  return coroutine.create(
    function()
      read()

      repeat
        next()
      until codepoint == -1

      yield("EOF")

      -- if #terminators > 0 then
      --   yield_error("Expected termination")
      -- end
    end
  )
end

return tokenizer
