local function temp()
  -- for line in input:lines() do
  --   row = row + 1
  --   col = 0

  --   local length = utf8.len(line)

  --   iterator = function()
  --     col = col + 1

  --     if col <= length then
  --       return utf8.codepoint(string, col)
  --     else
  --       return nil
  --     end
  --   end

  --   codepoint = iterator()

  --   while codepoint do
  --     proceed()
  --     codepoint = iterator()
  --   end
  -- end

  -- -- Is current expression empty (e.g. on new line)?
  -- local is_non_empty_expression = false

  -- -- Is current operation binary?
  -- local is_binop = false

  -- The codepoints buffer
  local buffer = {}

  local function buff()
    table.insert(buffer, codepoint)
    slash = false
  end

  local function yield(type)
    coroutine.yield{
      start_row = start_row,
      start_col = start_col,
      end_row = row,
      end_col = col,
      type = type,
      value = chars(buffer),
    }

    buffer = {}
  end

  -- The states stack
  local states = {}

  -- Current state
  local function state()
    if #states > 0 then
      local latest_state = states[#states]
      return latest_state
    else
      return nil
    end
  end

  local function push_state(new)
    table.insert(states, new)

    if table.includes({"id", "num", "text", "op"}, new) then
      buff()
    end
  end

  local function pop_state()
    table.remove(states)
  end

  local num_state = nil

  local function reset_num_state()
    num_state = {
      latest = nil,
      has_dot = false, -- Does the number have a dot?
      has_exponent = false, -- Does the number have an exponent?
      is_exponent = false, -- Is it currently an exponent?
      is_explicit = false, -- Do we expect the bitsize now?
    }
  end

  reset_num_state()

  -- The brackets stack
  local brackets = {}

  local function open_bracket()
    table.insert(brackets, reverse(codepoint))
    yield("BRACKET")
    push_state(nil)
  end

  local function close_bracket()
    if brackets[#brackets] == codepoint then
      table.remove(brackets)
      yield("BRACKET")
      pop_state()
    else
      yield_error("Unmatched closing bracket")
    end
  end

  local function is_texty()
    local cs = current_state()
    return cs == "text" or cs == "comment"
  end

  local function update()
    -- if is_whitespace() then
    --   -- Check if it currently is either text or comment literal
    --   if is_texty() then
    --     -- Append the whitespace to the buffer
    --     return buff()
    --   end

    -- elseif is_newline() then
    --   if not slash then
    --     if current_state() == "text" then
    --       -- Append the newline itself to the text literal
    --       buff()
    --     else
    --       -- Will be handled by state
    --     end
    --   elseif is_texty() then

    --   else
    --     slash = true
    --   end
    -- end

    if current_state() == nil then

      if is_alpha() then
        push_state("id")

      elseif is_digit() then
        push_state("num")

      elseif is_whitespace() then
        yield("SPACE")

      elseif is_newline() then
        yield("NEWLINE")

      elseif is_opening_bracket() then
        open_bracket()

      elseif is_closing_bracket() then
        close_bracket()

      elseif is_quote() then
        push_state("text")

      elseif is_comment() then
        push_state("comment")

      elseif is_backslash() then
        if backslash then
          yield_error("Unexpected explicit backslash")
        else
          yield("BSLASH")
          backslash = true
        end

      elseif is_colon() then
        yield_error("Unexpected colon")

      elseif is_semicolon() then
        yield("SMCOLON")

      else
        push_state("op")
      end

    elseif current_state() == "id" then

      -- Identifiers consist of Latin and Greek alphabet and digits.

      if is_alpha() or is_digit() then
        buff()
      else
        if keywords[chars(buffer)] then
          yield("KEYWORD")
        else
          yield("ID")
        end

        pop_state()

        if is_whitespace() or is_backslash() or is_colon() or
          is_semicolon() then
          push_state(nil)

          if is_backslash() then
            yield("BSLASH")
          elseif is_colon() then
            yield("COLON")
          elseif is_semicolon() then
            yield("SMCOLON")
          end

        elseif is_dot() then
          push_state("id")

        elseif is_opening_bracket() then
          open_bracket()

        elseif is_closing_bracket() then
          close_bracket()

        else
          push_state("op")
        end
      end

    elseif current_state() == "num" then

      -- Numbers consist of decimal digits.
      -- They can also be hexadecimal literals and `_i32` literals.

      if is_digit() then
        num_state.latest = nil
        buff()

      elseif is_dot() then
        if num_state.latest then
          yield_error("Unexpected dot")
        elseif num_state.has_dot then
          yield_error("Duplicate dot in a number")
        else
          num_state.latest = "."
          num_state.has_dot = true
          buff()
        end

      elseif is_underscore() then
        if num_state.latest then
          yield_error("Unexpected underscore")
        else
          num_state.latest = "_"
          buff()
        end

      elseif is("e") then
        if num_state.latest or num_state.has_exponent then
          yield_error("Unexpected exponent")
        else
          num_state.latest = "e"
          num_state.has_exponent = true
          buff()
        end

      elseif is("-") then
        -- Minus sign is only allowed for exponent
        if num_state.latest == "e" then
          num_state.latest = "-"
          buff()
        else
          yield_error("Unexpected token")
        end

      elseif is("f") or is("i") or is("u") then
        if (num_state.latest and not num_state.latest == "_") or
          num_state.is_explicit then
          yield_error("Unexpected token")
        else
          num_state.is_explicit = true
          num_state.latest = nil
          buff()
        end

      else
        if num_state.latest then
          yield_error("A number is not properly terminated")
        end

        yield("NUMBER")
        pop_state()

        if is_alpha() then
          yield_error("Unexpected identifier")

        elseif is_whitespace() then
          push_state(nil)

        elseif is_opening_bracket() then
          yield_error("Unexpected opening bracket")

        elseif is_closing_bracket() then
          close_bracket()

        elseif is_colon() then
          yield_error("Unexpected colon")

        else
          push_state("op")
        end
      end

    elseif current_state() == "text" then

      -- TODO: Interpolation?

      if is_backslash(codepoint) then
        slash = true

      elseif is_newline(codepoint) then
        if slash then
          -- Pretend there was no newline
        else
          table.insert(buffer, 10)
        end

      elseif codepoint == quotes[#quotes] then
        if slash then
          -- Append the quote as-is
          table.insert(buffer, codepoint)

          -- Discard the slash
          slash = false
        else
          -- Finish the text
          table.remove(quotes)
          yield("TEXT")
          set_state(nil)
        end

      else
        -- Continue reading the text
        table.insert(buffer, codepoint)

        -- Discard the slash
        slash = false
      end

    elseif current_state() == "op" then

      if is_alpha() or is_digit() or is_whitespace() or is_quote() or
        is_opening_bracket() or is_closing_bracket() then

        yield("OP")
        pop_state()

        if is_alpha() then
          push_state("id")

        elseif is_digit() then
          push_state("num")

        elseif is_whitespace() then
          -- Do nothing

        elseif is_quote() then
          -- TODO: ?

        elseif is_opening_bracket() then
          open_bracket()

        elseif is_closing_bracket() then
          close_bracket()

        else
          error "BUG: Unreacheable"
        end
      else
        buff()
      end

    elseif current_state() == "comment" then

      if is_newline() then
        yield("COMMENT")
        pop_state()
      else
        buff()
      end

    end

    if not is_backslash() then
      backslash = false
    end
  end
end
