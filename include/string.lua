function string:capitalize()
  self:gsub("^%l", string.upper)
end
