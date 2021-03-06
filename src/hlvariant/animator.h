/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@gmail.com>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>
            
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef HLVARIANT__ANIMATOR_H
#define HLVARIANT__ANIMATOR_H

#include "animation.h"
#include <KDebug>
#include "fwd.h"
#include "variantdata.h"
#include "namedsprite.h"
#include "animationfactory.h"
#include "common.h"

class NamedSprite;
class Point;
class AnimationFactory;

namespace HLVariant {

/**
  * A generic animator. 
  * Can be used as a base class for other specialized animators.
  */
template <typename _Variant>
class BaseAnimator {
public:
  typedef _Variant Variant;
protected:
  typedef typename UnwrappedGraphicalAPIPtr<Variant>::type API;
  typedef typename VariantData<Variant>::GameState GameState;
  typedef typename VariantData<Variant>::Move Move;
  typedef typename VariantData<Variant>::Piece Piece;
  API m_cinterface;
public:
  BaseAnimator(API cinterface)
    : m_cinterface(cinterface) { }
  virtual ~BaseAnimator() { }

  virtual AnimationGroupPtr warp(const GameState& final);
  virtual AnimationGroupPtr forward(const GameState& final, const Move& move);
  virtual AnimationGroupPtr back(const GameState& final, const Move& move);
};
  
  
template <typename _Variant>
class SimpleAnimator : BaseAnimator<_Variant> {
public:
  typedef _Variant Variant;
private:
  typedef BaseAnimator<Variant> Base;
protected:
  typedef typename Base::API API;
  typedef typename Base::GameState GameState;
  typedef typename Base::Move Move;
  typedef typename Base::Piece Piece;
  
  using Base::m_cinterface;
  
  virtual SchemePtr movement(const NamedSprite& sprite, const Point& from, const Point& to);
public:
  SimpleAnimator(API cinterface)
  : Base(cinterface) { }

  virtual AnimationGroupPtr warp(const GameState& final);
  virtual AnimationGroupPtr forward(const GameState& final, const Move& move);
  virtual AnimationGroupPtr back(const GameState& final, const Move& move);
};

// IMPLEMENTATION

template <typename Variant>
AnimationGroupPtr BaseAnimator<Variant>::warp(const GameState& final) {
  const GameState* current = m_cinterface->position();
  AnimationFactory res(m_cinterface->inner());
  
  for (int i = 0; i < current->board().size().x; i++) {
    for (int j = 0; j < current->board().size().y; j++) {
      Point p(i, j);  
      
      NamedSprite sprite = m_cinterface->getSprite(p);
      Piece piece = final.board().get(p);
    
      if (!sprite && piece != Piece()) {
        res.addPreAnimation(Animate::appear(m_cinterface->setPiece(p, piece, false)), 
                            Animate::Instant);
      }
      
      else if (sprite && piece == Piece()) {
        m_cinterface->takeSprite(p);
        res.addPreAnimation(Animate::disappear(sprite), Animate::Instant);
      }
      
      // sprite and piece differ
      else if (sprite && piece != Piece() && piece.name() != sprite.name()) {
        m_cinterface->takeSprite(p);
        res.addPreAnimation(Animate::morph(sprite, m_cinterface->setPiece(p, piece, false)), 
          Animate::Instant);
      }
    }
  }
  
  return res;
}

template <typename Variant>
AnimationGroupPtr BaseAnimator<Variant>::forward(const GameState& final, const Move&) {
  return warp(final);
}


template <typename Variant>
AnimationGroupPtr BaseAnimator<Variant>::back(const GameState& final, const Move&) {
  return warp(final);
}

template <typename Variant>
AnimationGroupPtr SimpleAnimator<Variant>::warp(const GameState& final) {
  AnimationFactory res(m_cinterface->inner());
  
  res.setGroup(Base::warp(final));
  
  return res;
}

template <typename Variant>
SchemePtr SimpleAnimator<Variant>::movement(const NamedSprite& sprite, const Point& from, const Point& to) {
  bool knight = m_cinterface->position()->board().get(from).type() == Piece::KNIGHT;
  int mtype = knight
    ? Animate::move::LShaped | Animate::move::Rotating 
    : Animate::move::Straight;
  return SchemePtr(new Animate::move(sprite, to, mtype));
}

template <typename Variant>
AnimationGroupPtr SimpleAnimator<Variant>::forward(const GameState& final, const Move& move) {
  AnimationFactory res(m_cinterface->inner());

  NamedSprite piece = m_cinterface->takeSprite(move.from());
  NamedSprite captured = m_cinterface->takeSprite(move.captureSquare());
  m_cinterface->setSprite(move.to(), piece);

  if (piece)
    res.addPreAnimation(*movement(piece, move.from(), move.to()));
  else
    kError() << "Scheduling movement animation for a null piece";

  if (captured)
    res.addPostAnimation(Animate::destroy(captured));

  if (move.promoteTo() != -1) {
    Piece promoted = final.board().get(move.to());

    if (promoted != Piece()) {
      QPoint real = m_cinterface->converter()->toReal(move.to());
      NamedSprite old_sprite = m_cinterface->getSprite(move.to());
      NamedSprite new_sprite = m_cinterface->setPiece(move.to(), promoted, /*false,*/ false);

      res.addPostAnimation(Animate::morph(old_sprite, new_sprite));
    }
    else
      kError() << "Scheduling morph animation for a null piece";
  }
  else if (move.kingSideCastling()) {
    Point rookSquare = move.to() + Point(1,0);
    Point rookDestination = move.from() + Point(1,0);

    NamedSprite rook = m_cinterface->takeSprite(rookSquare);
    m_cinterface->setSprite(rookDestination, rook);
    res.addPreAnimation(Animate::move(rook, rookDestination));
  }
  else if (move.queenSideCastling()) {
    Point rookSquare = move.to() + Point(-2,0);
    Point rookDestination = move.from() + Point(-1,0);

    NamedSprite rook = m_cinterface->takeSprite(rookSquare);
    m_cinterface->setSprite(rookDestination, rook);
    res.addPreAnimation(Animate::move(rook, rookDestination));
  }

  res.group()->addPostAnimation(warp(final));
  return res;
}

template <typename Variant>
AnimationGroupPtr SimpleAnimator<Variant>::back(const GameState& final, const Move& move) {
  AnimationFactory res(m_cinterface->inner());

  NamedSprite piece = m_cinterface->takeSprite(move.to());
  NamedSprite captured;
  Piece captured_piece = final.board().get(move.captureSquare());
  if (captured_piece != Piece()) {
    captured = m_cinterface->setPiece(move.captureSquare(), captured_piece, false);
    res.addPreAnimation(Animate::appear(captured));
  }

  if (!piece) {
    piece = m_cinterface->createPiece(move.to(), final.board().get(move.from()), false);
    res.addPreAnimation(Animate::appear(piece));
  }

  m_cinterface->setSprite(move.from(), piece);

  if (move.promoteTo() != -1) {
    Piece pawn_piece = final.board().get(move.from());
    if (pawn_piece != Piece()) {
      NamedSprite pawn = m_cinterface->createPiece(move.to(), pawn_piece, false);
      res.addPreAnimation(Animate::morph(piece, pawn));

      // replace piece with pawn
      m_cinterface->setSprite(move.from(), pawn);
      piece = pawn;
    }
  }
  else if (move.kingSideCastling()) {
    Point rookSquare = move.to() + Point(1,0);
    Point rookDestination = move.from() + Point(1,0);

    NamedSprite rook = m_cinterface->takeSprite(rookDestination);
    m_cinterface->setSprite(rookSquare, rook);

    res.addPreAnimation(Animate::move(rook, rookSquare));
  }
  else if (move.queenSideCastling()) {
    Point rookSquare = move.to() + Point(-2,0);
    Point rookDestination = move.from() + Point(-1,0);

    NamedSprite rook = m_cinterface->takeSprite(rookDestination);
    m_cinterface->setSprite(rookSquare, rook);

    res.addPreAnimation(Animate::move(rook, rookSquare));
  }
  
  res.addPreAnimation(*movement(piece, move.to(), move.from()));
  res.group()->addPostAnimation(warp(final));
  
  return res;

}

}

#endif // HLVARIANT__ANIMATOR_H
