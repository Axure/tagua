/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
            (c) 2007 Maurizio Monge <maurizio.monge@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef HLVARIANT__CHESS__SERIALIZER_H
#define HLVARIANT__CHESS__SERIALIZER_H

#include <boost/function.hpp>

#include "legalitycheck.h"
#include "san.h"
#include "icsverbose.h"

namespace HLVariant {
namespace Chess {

template <typename MoveGenerator>
class Serializer {
protected:
  typedef typename MoveGenerator::LegalityCheck LegalityCheck;
  typedef typename LegalityCheck::GameState GameState;
  typedef typename GameState::Move Move;
  typedef typename GameState::Board::Piece Piece;
  
  QString m_rep;

protected:
  virtual QString suffix(const Move& move, const GameState& ref);
  
  virtual QString san(const Move& move, const GameState& ref);
  
  virtual void minimal_notation(SAN& san, const GameState& ref);
  
  virtual Move get_san(const SAN& san, const GameState& ref);
  
  virtual QChar symbol(typename Piece::Type type) const;
  
  virtual Move parse_ics_verbose(const QString& str, const GameState& ref);
public:
  /** 
    * Create a serializer to a given string representation for moves.
    * \param rep A move representation type.
    */
  Serializer(const QString& rep);
  
  virtual ~Serializer();
  
  /**
    * Serialize a move given a reference game state.
    * We assume the move has already been tested against @a ref.
    * Calling this function on an untested or illegal move is safe but its return
    * value is undefined.
    * Do not try to call this function on an invalid move.
    * \param ref The position in which this move shall be executed.
    * \return A string representation for this move.
    */
  virtual QString serialize(const Move& move, const GameState& ref);
  
  /**
    * Convert a string representation of a move back to a move object.
    * \param ref The position in which this move shall be executed.
    * \return A move corresponding to the given string representation.
    */
  virtual Move deserialize(const QString& str, const GameState& ref);
};


// IMPLEMENTATION

template <typename MoveGenerator>
Serializer<MoveGenerator>::Serializer(const QString& rep)
: m_rep(rep) { }

template <typename MoveGenerator>
Serializer<MoveGenerator>::~Serializer() { }

template <typename MoveGenerator>
QString Serializer<MoveGenerator>::san(const Move& move, const GameState& ref) {
  if (!move.valid())
    return "";
    
  QString res;
      
  Piece piece = ref.board().get(move.from());
  Piece captured = ref.board().get(move.captureSquare());

  if (piece == Piece())
    return ""; // no piece in the initial square

  if (move.kingSideCastling()) {
    res = "O-O";
  }
  else if (move.queenSideCastling()) {
    res = "O-O-O";
  }
  else if (piece.type() == Piece::PAWN) {
    if (captured != Piece())
      res = move.from().col() + "x";

    res += move.to().toString(ref.board().size().y);
  }
  else {
    if (piece.type() != Piece::PAWN)
      res = symbol(piece.type()).toUpper();

    SAN tmp;
    tmp.from = move.from();
    tmp.to = move.to();
    tmp.type = piece.type();
    tmp.castling = SAN::NoCastling;
    minimal_notation(tmp, ref);

    res += tmp.from.toString(ref.board().size().y);
    if (captured != Piece())
      res += "x";
    res += tmp.to.toString(ref.board().size().y);
  }

  if (move.promoteTo() != -1)
    res += "=" + QString(symbol(static_cast<typename Piece::Type>(move.promoteTo())).toUpper());

  res += suffix(move, ref);

  return res;
}

template <typename MoveGenerator>
QString Serializer<MoveGenerator>::serialize(const Move& move, const GameState& ref) {
  if (m_rep == "simple") {
    int ysize = ref.board().size().y;
    QString res = move.from().toString(ysize) + move.to().toString(ysize);
    if (move.promoteTo() != -1)
      res = res + "=" + 
        symbol(
          static_cast<typename Piece::Type>(move.promoteTo())
        ).toUpper();
    return res;
  }
  else if (m_rep == "compact") {
    return san(move, ref);
  }
  else if (m_rep == "decorated") {
    QString res = san(move, ref);
    res.replace('K', "{king}");
    res.replace('Q', "{queen}");
    res.replace('R', "{rook}");
    res.replace('N', "{knight}");
    res.replace('B', "{bishop}");
    res.replace('P', "{pawn}");
    return res;
  }
  else {
    return "";
  }
}

template <typename MoveGenerator>
QString Serializer<MoveGenerator>::suffix(const Move& move, const GameState& ref) {
  GameState tmp(ref);
  tmp.move(move);
  
  MoveGenerator generator(tmp);
  if (generator.check(Piece::oppositeColor(ref.turn()))) {
    if (generator.stalled())
      return "#";
    else
      return "+";
  }
  else {
    return "";
  }
}

template <typename MoveGenerator>
typename Serializer<MoveGenerator>::Move 
Serializer<MoveGenerator>::get_san(const SAN& san, const GameState& ref) {
  Move candidate;

  if (san.invalid())
    return candidate;

  if (san.castling != SAN::NoCastling) {
    Point from = ref.kingStartingPosition(ref.turn());
    Point to = from + (san.castling == SAN::KingSide? Point(2,0) : Point(-2,0));
    Piece king = ref.board().get(from);
    if (king.type() != Piece::KING)
      return candidate;
    else {
      candidate = Move(from, to);
      LegalityCheck check(ref);
      if (check.legal(candidate))
        return candidate;
      else
        return Move();
    }
  }

  if (san.from.valid()) {
    candidate = Move(san.from, san.to, static_cast<typename Piece::Type>(san.promotion));
  }
  else {
    for (int i = 0; i < ref.board().size().x; i++) {
      for (int j = 0; j < ref.board().size().y; j++) {
        Point p(i, j);
        Piece piece = ref.board().get(p);
        
        Move mv(p, san.to, static_cast<typename Piece::Type>(san.promotion));
        if (p.resembles(san.from) && 
            piece.type() == san.type && 
            piece.color() == ref.turn()) {
  
          LegalityCheck check(ref);
          if (check.legal(mv))  {
            if (candidate.valid()) {
              // ambiguous!
              return Move();
            }
            else {
              // ok, we have found a candidate move
              candidate = mv;
            }
          }
        }
      }
    }
  }
  
  return candidate;
}

template <typename MoveGenerator>
typename Serializer<MoveGenerator>::Move 
Serializer<MoveGenerator>::deserialize(const QString& str, const GameState& ref) {
  if (m_rep == "compact") {
    SAN tmp;
    tmp.load(str, ref.board().size().y);
    return get_san(tmp, ref);
  }
  else if (m_rep == "ics-verbose") {
    return parse_ics_verbose(str, ref);
  }
  else {
    // no need to parse simple or decorated moves
    return Move();
  }
}


#define TRY(x) if(get_san(x, ref).valid()) return;
template <typename MoveGenerator>
void Serializer<MoveGenerator>::minimal_notation(SAN& san, const GameState& ref) {
  Point from = san.from;
  san.castling = SAN::NoCastling;

  // try notation without starting point
  san.from = Point::invalid();
  TRY(san);

  // add column indication
  san.from = Point(from.x, -1);
  TRY(san);

  // add row indication
  san.from = Point(-1, from.y);
  TRY(san);

  // add complete starting point
  san.from = from;
}
#undef TRY

template <typename MoveGenerator>
QChar Serializer<MoveGenerator>::symbol(typename Piece::Type type) const {
  if (type == Piece::KNIGHT)
    return 'n';
  else
    return Piece::typeName(type)[0];
}

template <typename MoveGenerator>
typename Serializer<MoveGenerator>::Move
Serializer<MoveGenerator>::parse_ics_verbose(const QString& str, const GameState& ref) {
  // here ref is the position _after_ this move
  ICSVerbose verbose;
  verbose.load(str, ref.board().size().y);
  
  Point from;
  Point to;
  
  if (verbose.castling == SAN::NoCastling) {
    from = verbose.from;
    to = verbose.to;
  }
  else {
    from = ref.kingStartingPosition(ref.previousTurn());
    to = from + (verbose.castling == SAN::KingSide ? Point(2,0) : Point(-2, 0));
  }

  return Move(from, to, static_cast<typename Piece::Type>(verbose.promotion));
}

} // namespace Chess
} // namespace HLVariant

#endif // HLVARIANT__CHESS__SERIALIZER_H

