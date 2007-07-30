import("piece_theme.lua")
import("shogi_themelib.lua")

shadow=7.0
shadow_color="#404050"
shadow_offset_x=6
shadow_offset_y=4
shadow_grow=5

theme.options = OptList {
  BoolOpt("moves_overlay", "Moves overlay", true)
}

function addChar(char, promoted)
  return function(i, size)
    i:draw_glyph(Rect(0,0,size,size), "Shogi.ttf", char,
		 promoted and "#d00000" or "#004000",
		 "#fff3c8", 4, false)
    return i
  end
end

function shogi_piece(char, white, promoted, ratio, ...)
  return addShadow(overlay(tile(white, ratio),
		    shogi_moves(...),
		    addChar(char, promoted)))
end

theme.black_king      = shogi_piece("0x6B", false, false, 1,
				    {-1,1},{0,1},{1,1},
				    {-1,0},{1,0},
				    {-1,-1},{0,-1},{1,-1})
theme.black_rook      = shogi_piece("0x72", false, false, 0.96,
				    {-1,0,1},{1,0,1},{0,-1,1},{0,1,1})
theme.black_p_rook    = shogi_piece("0x52", false, true, 0.96,
				    {-1,0,1},{1,0,1},{0,-1,1},{0,1,1},
				    {-1,-1},{1,-1},{-1,1},{1,1})
theme.black_bishop    = shogi_piece("0x62", false, false, 0.93,
				    {-1,-1,1},{1,-1,1},{-1,1,1},{1,1,1})
theme.black_p_bishop  = shogi_piece("0x42", false, true, 0.93,
				    {-1,-1,1},{1,-1,1},{-1,1,1},{1,1,1},
				    {-1,0},{1,0},{0,-1},{0,1})
theme.black_gold      = shogi_piece("0x67", false, false, 0.9,
				    {-1,1},{0,1},{1,1},
				    {-1,0},{1,0},{0,-1})
theme.black_silver    = shogi_piece("0x73", false, false, 0.9,
				    {-1,1},{0,1},{1,1},
				    {-1,-1},{1,-1})
theme.black_p_silver  = shogi_piece("0x53", false, true, 0.9,
				    {-1,1},{0,1},{1,1},
				    {-1,0},{1,0},{0,-1})
theme.black_knight    = shogi_piece("0x68", false, false, 0.86,
				    {-1,2},{1,2})
theme.black_p_knight  = shogi_piece("0x48", false, true, 0.86,
				    {-1,1},{0,1},{1,1},
				    {-1,0},{1,0},{0,-1})
theme.black_lance     = shogi_piece("0x6C", false, false, 0.83,
				    {0,1,1})
theme.black_p_lance   = shogi_piece("0x4C", false, true, 0.83,
				    {-1,1},{0,1},{1,1},
				    {-1,0},{1,0},{0,-1})
theme.black_pawn      = shogi_piece("0x70", false, false, 0.8,
				    {0,1})
theme.black_p_pawn    = shogi_piece("0x50", false, true, 0.8,
				    {-1,1},{0,1},{1,1},
				    {-1,0},{1,0},{0,-1})

theme.white_king      = shogi_piece("0x6B", true, false, 1,
				    {-1,1},{0,1},{1,1},
				    {-1,0},{1,0},
				    {-1,-1},{0,-1},{1,-1})
theme.white_rook      = shogi_piece("0x72", true, false, 0.96,
				    {-1,0,1},{1,0,1},{0,-1,1},{0,1,1})
theme.white_p_rook    = shogi_piece("0x52", true, true, 0.96,
				    {-1,0,1},{1,0,1},{0,-1,1},{0,1,1},
				    {-1,-1},{1,-1},{-1,1},{1,1})
theme.white_bishop    = shogi_piece("0x62", true, false, 0.93,
				    {-1,-1,1},{1,-1,1},{-1,1,1},{1,1,1})
theme.white_p_bishop  = shogi_piece("0x42", true, true, 0.93,
				    {-1,-1,1},{1,-1,1},{-1,1,1},{1,1,1},
				    {-1,0},{1,0},{0,-1},{0,1})
theme.white_gold      = shogi_piece("0x67", true, false, 0.9,
				    {-1,1},{0,1},{1,1},
				    {-1,0},{1,0},{0,-1})
theme.white_silver    = shogi_piece("0x73", true, false, 0.9,
				    {-1,1},{0,1},{1,1},
				    {-1,-1},{1,-1})
theme.white_p_silver  = shogi_piece("0x53", true, true, 0.9,
				    {-1,1},{0,1},{1,1},
				    {-1,0},{1,0},{0,-1})
theme.white_knight    = shogi_piece("0x68", true, false, 0.86,
				    {-1,2},{1,2})
theme.white_p_knight  = shogi_piece("0x48", true, true, 0.86,
				    {-1,1},{0,1},{1,1},
				    {-1,0},{1,0},{0,-1})
theme.white_lance     = shogi_piece("0x6C", true, false, 0.83,
				    {0,1,1})
theme.white_p_lance   = shogi_piece("0x4C", true, true, 0.83,
				    {-1,1},{0,1},{1,1},
				    {-1,0},{1,0},{0,-1})
theme.white_pawn      = shogi_piece("0x70", true, false, 0.8,
				    {0,1})
theme.white_p_pawn    = shogi_piece("0x50", true, true, 0.8,
				    {-1,1},{0,1},{1,1},
				    {-1,0},{1,0},{0,-1})

-- To be able to adapt this theme to chess too
-- should use a Free King (\u5954\u738b) instead 
theme.black_queen  = theme.black_gold
theme.white_queen  = theme.white_gold

