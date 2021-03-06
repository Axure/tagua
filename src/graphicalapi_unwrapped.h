/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@gmail.com>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef UNWRAPPEDGRAPHICALAPI_H
#define UNWRAPPEDGRAPHICALAPI_H

#include "graphicalapi.h"

/**
  * This class moves the interface that will be used by the animator to modify
  * tagua graphics into the variant's semantics.
  */
template <typename Variant>
class UnwrappedGraphicalAPI {
  GraphicalAPI* m_graphical_api;
public:
  typedef boost::shared_ptr<UnwrappedGraphicalAPI> Ptr;

  UnwrappedGraphicalAPI(GraphicalAPI* interface)
    : m_graphical_api(interface) {
  }

  virtual ~UnwrappedGraphicalAPI(){}

  virtual const PointConverter* converter() {
    return m_graphical_api->converter();
  }

  virtual const typename Variant::Position* position() {
    const WrappedPosition<Variant> *p = dynamic_cast<const WrappedPosition<Variant>*>(m_graphical_api->position().get());
    if(p)
      return &p->inner();
    else {
      MISMATCH(m_graphical_api->position(), WrappedPosition<Variant>);
      return NULL;
    }
  }

  virtual NamedSprite setPiece(const Point& p, const typename Variant::Piece& piece, bool show) {
    WrappedPiece<Variant> wpiece(piece);
    return m_graphical_api->setPiece(p, &wpiece, show);
  }

  virtual NamedSprite createPiece(const Point& p, const typename Variant::Piece& piece, bool show) {
    WrappedPiece<Variant> wpiece(piece);
    return m_graphical_api->createPiece(p, &wpiece, show);
  }

  virtual void setSprite(const Point& p, const NamedSprite& s) {
    m_graphical_api->setSprite(p, s);
  }

  virtual NamedSprite getSprite(const Point& p) {
    return m_graphical_api->getSprite(p);
  }

  virtual NamedSprite takeSprite(const Point& p) {
    return m_graphical_api->takeSprite(p);
  }

  virtual int poolSize(int pool) const {
    return m_graphical_api->poolSize(pool);
  }

  virtual NamedSprite insertPoolPiece(int pool, int index, const typename Variant::Piece& piece) {
    WrappedPiece<Variant> wpiece(piece);
    return m_graphical_api->insertPoolPiece(pool, index, &wpiece);
  }

  // virtual NamedSprite getPoolSprite(int pool, int index) {
  //   return m_graphical_api->getPoolSprite(pool, index);
  // }

  virtual void removePoolSprite(int pool, int index) {
    m_graphical_api->removePoolSprite(pool, index);
  }

  virtual NamedSprite takePoolSprite(int pool, int index) {
    return m_graphical_api->takePoolSprite(pool, index);
  }

  virtual std::pair<int, int> droppedPoolPiece() {
    return m_graphical_api->droppedPoolPiece();
  }

  virtual AnimationPtr animate(const Animate::Scheme& scheme, Animate::AnimationType type = Animate::Normal) {
    return m_graphical_api->animate(scheme, type);
  }

  GraphicalAPI* inner() const { return m_graphical_api; }
};

#endif //UNWRAPPEDGRAPHICALAPI_H
