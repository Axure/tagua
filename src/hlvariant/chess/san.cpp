/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
            (c) 2007 Maurizio Monge <maurizio.monge@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "san.h"
#include "piece.h"

namespace HLVariant {
namespace Chess {

//                         1           2                   3
QRegExp SAN::pattern("^([PRNBKQ])?([a-wyzA-Z]?\\d*|x\\d+)([-x@])?"
//                         4         5        6
                     "([a-zA-Z]\\d+)(=?([RNBKQrnbkq]))?[+#]?[\?!]*");
QRegExp SAN::kingCastlingPattern("^[oO0]-?[oO0][+#]?");
QRegExp SAN::queenCastlingPattern("^[oO0]-?[oO0]-?[oO0][+#]?");
QRegExp SAN::nonePattern("^none");

SAN::SAN()
: from(Point::invalid())
, to(Point::invalid())
, type(-1)
, promotion(-1)
, castling(NoCastling)
, drop(false) {
}

int SAN::getType(const QString& letter) {
  if (letter.isEmpty())
    return Piece::PAWN;
    
  switch(letter[0].toLower().toAscii()) {
  case 'k':
    return Piece::KING;
  case 'q':
    return Piece::QUEEN;
  case 'r':
    return Piece::ROOK;
  case 'n':
    return Piece::KNIGHT;
  case 'b':
    return Piece::BISHOP;
  case 'p':
    return Piece::PAWN;
  default:
    return Piece::INVALID_TYPE;
  }
}

void SAN::load(const QString& str, int& offset, int ysize) {
  if (nonePattern.indexIn(str, offset, QRegExp::CaretAtOffset) != -1) {
    from = Point::invalid();
    to = Point::invalid();
    offset += nonePattern.matchedLength();
  }
  else if (pattern.indexIn(str, offset, QRegExp::CaretAtOffset) != -1) {
    type = getType(pattern.cap(1));
    drop = pattern.cap(3) == "@";
    if (drop)
      from = Point::invalid();
    else
      from = Point(pattern.cap(2), ysize);
    to = Point(pattern.cap(4), ysize);
    promotion = pattern.cap(6).isEmpty() ? -1 : getType(pattern.cap(6));
    castling = NoCastling;
    offset += pattern.matchedLength();
  }
  else if (queenCastlingPattern.indexIn(str, offset, QRegExp::CaretAtOffset) != -1) {
    castling = QueenSide;

    offset += queenCastlingPattern.matchedLength();
  }
  else if (kingCastlingPattern.indexIn(str, offset, QRegExp::CaretAtOffset) != -1) {
    castling = KingSide;

    offset += kingCastlingPattern.matchedLength();
  }
  else {
    //kDebug() << "error!!!! " << str.mid(offset);
    to = Point::invalid();
  }
}

void SAN::load(const QString& str, int ysize) {
  int offset = 0;
  load(str, offset, ysize);
}

QDebug operator<<(QDebug os, const SAN& move) {
  if (move.castling == SAN::KingSide)
    os << "O-O";
  else if (move.castling == SAN::QueenSide)
    os << "O-O-O";
  else
    os << move.type << ": " << move.from << " -> " << move.to;
  return os;
}


} // namespace Chess
} // namespace HLVariant


