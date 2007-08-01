/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@sns.it>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "variants.h"
#include "tagua.h"
#include "variants/chess.h"
#include "variants/crazyhouse.h"
#include "variants/shogi.h"
#include "hlvariant/chess/variant.h"
#include "hlvariant/crazyhouse/variant.h"
#include "hlvariant/minichess5/variant.h"

// remove me
namespace HLVariant {
namespace Crazyhouse {
class Variant;
}
}

#if 0
#include "variants/atomic.h"
#include "variants/king4pawns.h"
#include "variants/progressive.h"
#include "variants/reversi.h"
#include "variants/connect4.h"
#include "variants/chainreaction.h"
#include "variants/dummy.h"
#include "variants/xiangqi.h"
#endif

bool Variant::s_initialized = false;

Variant::Variants Variant::s_registered;

void Variant::register_variant(VariantInfo* v) {
  s_registered[v->name()] = v;
}

template<typename T>
void Variant::register_variant() {
  register_variant(T::info());
}

void Variant::initialize() {
  register_variant<ChessVariant>();
  register_variant<CrazyhouseVariant>();
  // register_variant<AtomicVariant>();
  // register_variant<King4PawnsVariant>();
  // register_variant<ProgressiveVariant>();
  // register_variant<ReversiVariant>();
  // register_variant<Connect4Variant>();
  // register_variant<ChainReactionVariant>();
  // register_variant<DummyVariant>();
  register_variant<ShogiVariant>();
  // register_variant<XiangQiVariant>();
  register_variant<HLVariant::Chess::Variant>();
  register_variant<HLVariant::Crazyhouse::Variant>();
  register_variant<HLVariant::Minichess5::Variant>();
  s_initialized = true;
}

VariantInfo* Variant::variant(const QString& name) {
  if(!s_initialized)
    initialize();

  Variants::iterator it = s_registered.find(name);
  return it == s_registered.end() ? NULL : it->second;
}

const Variant::Variants& Variant::allVariants(){
  if(!s_initialized)
    initialize();

  return s_registered;
}
