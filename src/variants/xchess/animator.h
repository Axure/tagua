/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@sns.it>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>
            
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <boost/shared_ptr.hpp>
#include "animation.h"
#include "fwd.h"

class NamedSprite;
class Point;

/**
  * A generic animator. 
  * Can be used as a base class for other specialized animators.
  */
template <typename Variant>
class SimpleAnimator {
  typedef typename UnwrappedGraphicalAPIPtr<Variant>::type API;
  typedef typename Variant::Position Position;
  typedef typename Variant::Move Move;
  typedef typename Variant::Piece Piece;
  API m_cinterface;
protected:
  void updatePool(const Position& final);
  
  virtual SchemePtr movement(const NamedSprite& sprite, const Point& from, const Point& to);
public:
  SimpleAnimator(API cinterface)
    : m_cinterface(cinterface) {
  }
  virtual ~SimpleAnimator() { }

  AnimationGroupPtr warp(const Position& final);
  AnimationGroupPtr forward(const Position& final, const Move& move);
  AnimationGroupPtr back(const Position& final, const Move& move);
};


#endif // ANIMATOR_H
