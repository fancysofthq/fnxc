local ansi = require("include/ansi")

local function describe(name, descriptor)
  local start_at = os.clock()
  print("Running " .. name .. " specs...\n")

  local specs = {}

  function it(assertion, code)
    local status = xpcall(code, function(err)
      table.insert(specs, {assertion = assertion, error = err})
    end)

    if status then
      table.insert(specs, {assertion = assertion})
    end
  end

  local status = xpcall(descriptor, function(err)
    table.insert(errors, debug.traceback(err))
  end, it)

  local end_at = os.clock()
  local err = 0

  for _, spec in ipairs(specs) do
    if spec.error then
      err = err + 1

      print(
        ansi.red .. ansi.bright .. "F " .. ansi.reset_all .. "It " ..
          spec.assertion)
      print("  " .. ansi.red .. spec.error .. ansi.reset)
    else
      print(
        ansi.green .. ansi.bright .. "S " .. ansi.reset_all .. "It " ..
          spec.assertion)
    end
  end

  local color = err > 0 and ansi.red or ansi.green

  print(string.format(ansi.reset_all .. "\nPassed " .. ansi.bright ..
                        color .. "%d/%d" .. ansi.reset_all ..
                        " %s specs in %.3fs", (#specs - err), #specs,
                      name, end_at - start_at))
end

return describe
