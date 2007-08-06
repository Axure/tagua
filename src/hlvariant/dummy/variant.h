/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@sns.it>
            (c) 2007 Maurizio Monge <maurizio.monge@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef HLVARIANT__DUMMY__VARIANT_H
#define HLVARIANT__DUMMY__VARIANT_H

#include "../crazyhouse/variant.h"
#include "legalitycheck.h"
#include "movegenerator.h"

namespace HLVariant {
namespace Dummy {

struct TAGUA_EXPORT Variant {
  typedef Crazyhouse::Variant::Move Move;
  typedef Crazyhouse::Variant::GameState GameState;
  typedef LegalityCheck<GameState> LegalityCheck;
  typedef MoveGenerator<LegalityCheck> MoveGenerator; // to make VariantData happy
  typedef Crazyhouse::Serializer<MoveGenerator> Serializer;
  typedef DropAnimatorMixin<SimpleAnimator<Variant> > Animator;
  typedef Crazyhouse::MoveFactory<GameState> MoveFactory;
  
  static const bool hasICS = true;
  static const bool m_simple_moves = false;
  static const char *m_name;
  static const char *m_theme_proxy;

  static int moveListLayout() { return 0; }

  static VariantInfo* static_variant;
  static VariantInfo* info();
  static OptList positionOptions() { return OptList(); }
};

} // namespace Dummy
} // namespace HLVariant

#endif // HLVARIANT__DUMMY__VARIANT_H

