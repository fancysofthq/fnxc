local original_require = require
local idempotent_modules = {"math", "string", "utf8", "table"}
local idempotent_functions = {["os"] = {"difftime"}}

-- Rewrite the original `require` to control idempotency.
function require(arg)
  print("Got ya! Arg = " .. arg)
  return original_require(arg)
end

nx.file.caching = "full" -- Always cache, even for diff. targets
nx.file.caching = "target" -- Cache per target
nx.file.caching = "none" -- Disable caching for this file
nx.file.caching = function(arg)
  -- Run this function to determine caching
end

nx.file.cache = function(old_target)
  return nx.target:diff(old_target) and nx.target
end

-- If we have a file named `.config` with `foo` field
-- `nil` means "do not rebuild the file"
-- Truthy value means "update the cache"
nx.file.cache = function(old_foo)
  file = open(".config")

  if not file.foo == old_foo then
    return file.foo
  else
    return nil
  end
end
