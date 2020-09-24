return {
  -- Return a number of octets (8 bits)
  -- needed to store a natural number *num*.
  --
  -- ```nx
  -- # 1 × 8 bits is needed to store number 20.
  -- @assert({{ nx.util.octfor(20) }} == 1)
  --
  -- # 1 × 8 bits is needed to store number 255.
  -- @assert({{ nx.util.octfor(255) }} == 1)
  --
  -- # 2 × 8 bits is needed to store number 256.
  -- @assert({{ nx.util.octfor(256) }} == 2)
  -- ```
  octfor = function(num)
    return math.floor(2 ^ (math.ceil(math.log(num + 1, 2) / 8) - 1))
  end,

  -- Because `math` does not have it.
  round = function(num, zeros)
    local mult = 10 ^ (zeros or 0)
    local res = math.floor(num * mult + 0.5) / mult

    if (zeros or 0) > 0 then
      return res
    else
      return math.floor(res)
    end
  end
}
