import("piece_theme.lua")
import("selection.lua")

theme.background = function(size)
  local dark_square = "#319231"
  local light_square = "#cecf9c"
  local i = Image(size*2,size*2)
  i:fill_rect(Rect(0,0,size,size), light_square)
  i:fill_rect(Rect(size,0,size,size), dark_square)
  i:fill_rect(Rect(0,size,size,size), dark_square)
  i:fill_rect(Rect(size,size,size,size), light_square)
  return i
end

theme.validmove = fromColor("#bdaede")

