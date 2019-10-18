local reader = {}

-- In iterator over file contents, char by char.
function reader.read(file)
  return coroutine.create(function()
    local row = 0
    local col = 0
    local first_line = true

    for line in file:lines() do
      if not first_line then
        coroutine.yield{row = row, col = col + 1, codepoint = 10} -- 10 is newline
      end

      first_line = false
      row = row + 1
      col = 1

      for column, codepoint in utf8.codes(line) do
        col = column
        coroutine.yield{row = row, col = col, codepoint = codepoint}
      end
    end
  end)
end

return reader
