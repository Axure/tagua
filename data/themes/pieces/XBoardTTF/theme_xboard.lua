import("piece_theme.lua")
function fromGlyph(glyph)
  return fromFontGlyph("XBoard.ttf", glyph, "black", "#ffffce")
end

theme.black_bishop = fromGlyph("0x0042")
theme.black_king   = fromGlyph("0x004B")
theme.black_knight = fromGlyph("0x004E")
theme.black_pawn   = fromGlyph("0x0050")
theme.black_queen  = fromGlyph("0x0051")
theme.black_rook   = fromGlyph("0x0052")

theme.white_bishop = fromGlyph("0x0062")
theme.white_king   = fromGlyph("0x006B")
theme.white_knight = fromGlyph("0x006E")
theme.white_pawn   = fromGlyph("0x0070")
theme.white_queen  = fromGlyph("0x0071")
theme.white_rook   = fromGlyph("0x0072")

