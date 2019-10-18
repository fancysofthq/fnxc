require("include/table")

function utf8.split(string, delimiter)
  local result = {}
  local from = 1
  local delim_from, delim_to = string.find(string, delimiter, from)

  while delim_from do
    table.insert(result, string.sub(string, from, delim_from - 1))
    from = delim_to + 1
    delim_from, delim_to = string.find(string, delimiter, from)
  end

  table.insert(result, string.sub(string, from))

  return result
end

-- Turn an *array* of codepoints into an UTF-8 text.
function utf8.chars(array)
  local buffer = ""

  for v in table.values(array) do
    buffer = buffer .. utf8.char(v)
  end

  return buffer
end
