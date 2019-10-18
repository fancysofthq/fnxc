local tokenizer = {}

-- Return `true` if *codepoint* is in either Latin or Greek alphabet.
local function is_alpha(codepoint)
  return -- Latin upcases (A-Z)
  (codepoint >= 65 and codepoint <= 90) or
    -- Latin lowercases (a-z)
    (codepoint >= 97 and codepoint <= 122) or
    -- Greek upcases #1 (Α-Ρ)
    (codepoint >= 913 and codepoint <= 929) or
    -- Greek upcases #2 (Σ-Ω)
    (codepoint >= 931 and codepoint <= 937) or
    -- Greek lowercases (α-ω)
    (codepoint >= 945 and codepoint <= 969)
end

-- Return `true` if *codepoint* is a digit.
local function is_digit(codepoint)
  return (codepoint >= 48 and codepoint <= 57)
end

-- Match on "([{".
local function is_opening_bracket(codepoint)
  return (codepoint == 40 or codepoint == 91 or codepoint == 123)
end

-- Match on ")]}".
local function is_closing_bracket(codepoint)
  return (codepoint == 41 or codepoint == 93 or codepoint == 125)
end

local function is_quote(codepoint)
  return (codepoint == 34 or codepoint == 39 or codepoint == 96)
end

local keywords = {
  ["require"] = true,
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
  elsif = true,
  ["else"] = true,
  ["while"] = true,
  ["until"] = true,
  next = true,
  ["break"] = true,
  case = true,
  when = true,
  ["yield"] = true,
}

local percent_symbols = {
  q = true, -- A quoted string, probably including `"` glyph
  Q = true, -- Interpolation-enabled string
  w = true, -- Words (slice of space-separated strings)
  W = true, -- Interpolation-enabled words
  t = true, -- Twine, probably including the `'` glyph
  d = true, -- Dictionary (slice of space-separated twines)
  i = true, -- Slice of symbols
  g = true, -- Slice of glyphs, probably including the ` itself
}

-- The main tokenizer function. Yields tokens one-by-one.
function tokenizer.tokenize(input)
  local row = 0 -- Current row
  local col = 0 -- Current column

  local row_start = 0 -- At which row the token began
  local col_start = 0 -- At which column the token began

  local source = "" -- Current token value

  local identifier = false -- Is current token an identifier like `foo`
  local op = false -- Is current token an op (either binary or unary)
  local binop = false -- Is current operation binary?
  local number = false -- Is current token a number
  local slash = false -- After `\`
  local newline = false -- Is there recent newline?

  local quotes = {} -- A stack of codepoints to end the current TEXT
  local interpolation = false -- Allow interpolation?

  local percent = false -- Is the last char `%`?
  local waiting_for_percent_delimeter = false
  local building_heredoc = false
  local heredoc = nil -- A string to end a heredoc (separator)

  return coroutine.create(function()
    -- Yield a token.
    local function yield(type, value)
      coroutine.yield{
        row_start = row_start,
        col_start = col_start,
        row_end = row,
        col_end = col,
        type = type,
        value = value,
      }

      -- print("< " .. type .. "\t" .. (value or ""))

      source = ""
    end

    local function yield_error(message)
      assert(message, "Message is required")

      coroutine.yield{
        row_start = row_start,
        col_start = col_start,
        row_end = row,
        col_end = col,
        error = message,
      }
    end

    -- Yield all pending tokens. Typically called on newline.
    local function yield_all()
      if identifier then
        if keywords[source] then
          yield("KEYWORD", source)
        else
          yield("ID", source)
        end

        identifier = false
      elseif op then
        if binop then
          yield("BINOP", source)
          op = false
        else
          -- An orphan unop in the end of a line is prohibited
          yield_error("Unexpected orphan unop `" .. source .. "`")
        end
      elseif number then
        yield("NUMBER", source)
        number = false
      end
    end

    for line in input:lines() do
      row = row + 1

      for column, codepoint in utf8.codes(line) do
        col = column
        newline = false

        -- print("> " .. utf8.char(codepoint))

        if #quotes > 0 then
          if codepoint == quotes[#quotes] then
            yield("TEXT", source)
            yield("DELIM", string.char(codepoint))
            table.remove(quotes)
          elseif string.char(codepoint) == "\\" then
            slash = true
          elseif slash and string.char(codepoint) == "\n" then
            -- Do not append the newline
            slash = false
          else
            if slash then
              source = source .. "\\"
              slash = false
            end

            source = source .. string.char(codepoint)
          end

          goto continue
        end

        if slash and not string.char(codepoint) == " " then
          yield_error("Expected newline")
        end

        if is_alpha(codepoint) then
          if op then
            -- Operations cannot contain alpha chars
            op = false

            if percent then
              if percent_symbols[string.char(codepoint)] then
                yield("PERCENT", string.char(codepoint))
                waiting_for_percent_delimeter = true
              else
                -- It may be a heredoc then!
                building_heredoc = true
              end

              source = source .. utf8.char(codepoint)
              goto continue
            elseif binop then
              yield("BINOP", source)
              binop = false
            else
              yield("UNOP", source)
            end
          end

          if not identifier then
            identifier = true
            ignore_newline = false
            row_start = row
            col_start = col
          end

          source = source .. utf8.char(codepoint)

          goto continue
        elseif identifier then
          if is_digit(codepoint) then
            -- Identifiers can include digits
            source = source .. utf8.char(codepoint)
            goto continue
          elseif string.char(codepoint) == ":" then
            yield("NARG", source)
            identifier = false

            goto continue
          else
            if keywords[source] then
              yield("KEYWORD", source)
            else
              yield("ID", source)
              binop = true
            end

            identifier = false
          end
        end

        if is_digit(codepoint) then
          -- Operations cannot contain digits
          if op then
            op = false

            if binop then
              yield("BINOP", source)
              binop = false
            else
              yield("UNOP", source)
            end
          end

          if not number then
            number = true
            ignore_newline = false
            row_start = row
            col_start = col
          end

          source = source .. utf8.char(codepoint)

          goto continue
        elseif number then
          yield("NUMBER", source)
          number = false
          binop = true
        end

        if string.char(codepoint) == " " then
          if op then
            if binop then
              yield("BINOP", source)
              op = false
              binop = false
            else
              yield_error("Unexpected binop `" .. source .. "`")
            end
          end
        elseif is_opening_bracket(codepoint) then
          if op then
            op = false

            if binop then
              yield("BINOP", source)
              binop = false
            else
              yield("UNOP", source)
            end
          end

          yield("DELIM", string.char(codepoint))
          binop = false
        elseif is_closing_bracket(codepoint) then
          if op then
            yield_error("Unexpected op `" .. string.char(codepoint) ..
                          "` after closing bracket")
          end

          yield("DELIM", string.char(codepoint))
        elseif string.char(codepoint) == "," then
          yield("COMMA")
          binop = false
        elseif string.char(codepoint) == ":" then
          yield("COLON")
          binop = false
        elseif is_quote(codepoint) then
          yield("DELIM", string.char(codepoint))
          table.insert(quotes, codepoint)
          source = ""
        elseif string.char(codepoint) == "\\" then
          -- Slash means "treat the next newline as the same line",
          -- it must be preserved for formatting.
          -- It's a syntax error to have a non-newline symbol afterwards.
          yield("SLASH")
          slash = true
        else
          if op or number then
            source = source .. utf8.char(codepoint)
          else
            if string.char(codepoint) == "%" then
              percent = true
              waiting_for_percent_delimeter = false
              building_heredoc = false
            end

            op = true
            ignore_newline = false
            source = utf8.char(codepoint)
          end
        end

        ::continue::
      end

      yield_all()

      if slash then
        slash = false
      elseif not newline then
        yield("NEWLINE")
        newline = true
      end
    end

    col = col + 1
    yield_all()
  end)
end

return tokenizer
