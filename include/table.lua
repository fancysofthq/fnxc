-- Replace the latest value in the table.
function table.replace(table, value)
  table.remove(table)
  table.insert(table, value)
end

-- Iterate through table values.
function table.values(table)
  local i = 0

  return function()
    i = i + 1
    return table[i]
  end
end

-- Check if the table includes the *value*.
function table.includes(table, value)
  for element in table:values() do
    if value == element then
      return true
    end
  end

  return false
end

-- Find bad element in table, i.e. one which is not
-- in the *allowed* list and return its index.
function table.odd_value(tbl, allowed)
  for i, e in ipairs(tbl) do
    if not table.includes(allowed, e) then
      return i
    end
  end

  return nil
end

-- Returns *n*th element of table *t* matching comparator *cmp*.
-- If comparator is a function, it is called on each element,
-- otherwise a simple comparison happens.
--
-- ```
-- local t = {42, 43, 44, 45}
-- table.nth(t, 2, function(_, v)
--   if v > 42 then
--     return true
--   end
-- end) == {3 = 44}
-- ```
function table.nth(t, n, cmp)
  local i = 0

  for key, value in pairs(t) do
    if type(cmp) == "function" then
      local match = cmp(k, v)
    else
      local match = v == cmp
    end

    if match then
      i = i + 1

      if i >= n then
        return k, v
      end
    end
  end
end

-- Find **first** matching element in table *t* by comparator *cmp*.
-- If comparator is a function, it is called on each element,
-- otherwise a simple comparison happens.
function table.find(t, cmp)
  return table.nth(t, 1, cmp)
end

function table.find_by_key(t, key)
  return table.find(
    t,
    function(k)
      if k == key then
        return true
      end
    end
  )
end
