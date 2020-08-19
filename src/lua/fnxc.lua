fnxc = {
  llvm_type = function (type)
    if type.has_alias("FBin32") then
      return "float"
    elseif type.has_alias("FBin64") then
      return "double"
    else
      -- TODO:
      nx.panic("Can not map " .. type ..
        " to an LLVM floating-point type")
    end
  end
}
