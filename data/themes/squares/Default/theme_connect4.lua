import("piece_theme.lua")
import("selection.lua")

theme.background = function(size)
  local upper = Image(size,size)
  local lower = Image(size,size)
  upper:fill_rect(Rect(0,0,size,size), "#8090a0")
  upper:draw_svg(Rect(0,0,size,size), "arrow_grey.svg")
  lower:fill_rect(Rect(0,0,size,size), "#2030d0")
  lower:draw_svg(Rect(0,0,size,size), "spiral.svg")
  return { [Rect(0,0,size*7,size)] = upper, [Rect(0,size,size*7,size*6)] = lower }
end

theme.highlighting = fromSVG("arrow_yellow.svg")
theme.validmove = fromColor("#bdaede")

