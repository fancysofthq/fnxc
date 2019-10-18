local tokenizer = require("src/tokenizer")

local input = "foo(bar + baz)"
local file = io.tmpfile()
file:write(input)
file:seek("set", 0)
local coro = tokenizer.tokenize(file)

while true do
  local _, token = coroutine.resume(coro)

  if _ and token then
    print(token.type .. "\t" .. tostring(token.value))
  elseif _ then
    break
  else
    error(token)
  end
end
