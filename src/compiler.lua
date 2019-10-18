local path = "./temp"

local file = io.open(path, "r")
assert(file, "The file must be opened")

local tokenizer = require("src/tokenizer").tokenize(file)

while true do
  local _, token = coroutine.resume(tokenizer)

  if not token then
    break
  end

  if token.error then
    error("at " .. path .. " " .. token.row_start .. ":" ..
            token.col_start .. " → " .. token.row_end .. ":" ..
            token.col_end .. " — " .. token.error)
  end

  print(token.row_start .. ":" .. token.col_start .. " → " ..
          token.row_end .. ":" .. token.col_end .. "\t" .. token.type ..
          "\t" .. token.value)
end

