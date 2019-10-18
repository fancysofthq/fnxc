local codes = {
  reset_all = 0,
  reset_bright = 21,
  reset_dim = 22,
  reset_underlined = 24,
  reset_blink = 25,
  reset_reverse = 27,
  reset_hidden = 28,

  -- Miscellaneous
  bright = 1,
  dim = 2,
  underline = 4,
  blink = 5,
  reverse = 7,
  hidden = 8,

  -- Foreground colors
  reset = 39,
  black = 30,
  red = 31,
  green = 32,
  yellow = 33,
  blue = 34,
  magenta = 35,
  cyan = 36,
  light_gray = 37,
  dark_gray = 90,
  light_red = 91,
  light_green = 92,
  light_yellow = 93,
  light_blue = 94,
  light_magenta = 95,
  light_cyan = 96,
  white = 97,

  -- Background colors
  bg_reset = 49,
  bg_black = 40,
  bg_red = 41,
  bg_green = 42,
  bg_yellow = 43,
  bg_blue = 44,
  bg_magenta = 45,
  bg_cyan = 46,
  bg_light_gray = 47,
  bg_dark_gray = 100,
  bg_light_red = 101,
  bg_light_green = 102,
  bg_light_yellow = 103,
  bg_light_blue = 104,
  bg_light_magenta = 105,
  bg_light_cyan = 106,
  bg_white = 107,
}

local format = string.char(27) .. '[%dm'

for key, value in pairs(codes) do
  codes[key] = string.format(format, value)
end

return codes
