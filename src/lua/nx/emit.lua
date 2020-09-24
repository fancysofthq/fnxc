-- Can pass a function, which is useful when you want to wrap
-- an implicitly emitting code. For example:
--
-- ```nx
-- {% nx.emit(function() do %}
--   do_{{ strange }}_stuff()
-- {% end) %}
-- ```
return function(value)
  if type(value) == "function" then
    value()
  else
    c_emit(value:dump()) -- TODO:
  end
end
