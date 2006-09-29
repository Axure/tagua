/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@sns.it>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "icsentity.h"
#include "game.h"
#include "algebraicnotation.h"
#include "icsconnection.h"
#include "positioninfo.h"
#include "poolinfo.h"
#include <iostream>

using namespace boost;

ICSEntity::ICSEntity(VariantInfo* variant, const shared_ptr<Game>& game,
                   int side, int gameNumber,
                   const shared_ptr<ICSConnection>& connection, AgentGroup* group)
: GameBasedEntity(game)
, m_variant(variant)
, m_connection(connection)
, m_side(side)
, m_game_number(gameNumber)
, m_dispatcher(group, this) { }

AbstractPosition::Ptr ICSEntity::position() const {
  return m_game->position();
}

void ICSEntity::executeMove(AbstractMove::Ptr) { }

void ICSEntity::updateGame(const Index& index, AbstractMove::Ptr icsMove,
                                         AbstractPosition::Ptr icsPos) {
  if(index>0 && m_game->containsIndex(index.prev()) && icsMove) {
    if(AbstractPosition::Ptr position = m_game->position(index.prev())) {
      position = position->clone();

      if(position->testMove(icsMove)) {
        position->move(icsMove);
        icsPos->copyPoolFrom(position);
        if(!position->equal(icsPos))
          std::cout << "[inconsistency] computed position differs from expected!" << std::endl;
      }
      else
        std::cout << "[inconsistency] invalid move from server!" << std::endl;
    }
  }
  if(m_game->lastMainlineIndex() > index)
    m_game->truncate(index);
  m_game->insert(icsMove, icsPos, index);
  m_game->goTo(index);
  m_dispatcher.move(icsMove, icsPos);
}

ICSEntity::UpdateType ICSEntity::getUpdate(const Index& index) {
  std::pair<int, int> steps = m_game->index().stepsTo(index);
  if (steps.first == 0) {
    if (steps.second == 0) return NoMove;
    else if (steps.second == 1) return MoveForward;
    else return Forward;
  }
  else if (steps.first == 1 && steps.second == 0) return MoveBack;
  else if (steps.second == 0) return Back;

  return NonComparableIndexes;
}

void ICSEntity::notifyStyle12(const PositionInfo& style12, bool is_starting) {
  if (style12.gameNumber != m_game_number) return;

  m_dispatcher.clockUpdate(style12.whiteTime, style12.blackTime);

  // get last move verbose notation
  AbstractMove::Ptr last_move;
  VerboseNotation last_move_verbose_notation(style12.lastMove, style12.position->size().y);
  if(!is_starting && last_move_verbose_notation.valid())
    last_move = m_variant->getVerboseMove(
                  style12.position->previousTurn(),
                  last_move_verbose_notation);

  if(style12.index()>0 && m_game->containsIndex(style12.index()-1)
                  && last_move && m_variant->name() != "Dummy") {
    AbstractPosition::Ptr position = m_game->position(style12.index()-1);
    if(position) {
      AlgebraicNotation last_move_alg_notation(style12.lastMoveSAN, style12.position->size().y);
      AbstractMove::Ptr mv = position->getMove(last_move_alg_notation);
      if (!mv || !mv->equals(last_move)) {
        std::cout <<
          "[server inconsistency] SAN for last move is different from verbose notation"
          << std::endl;
      }
    }
  }
  if(style12.index()>0 && m_variant->name() != "Dummy"
        && (!m_game->containsIndex(style12.index()-1) || !m_game->position(style12.index()-1)) )
    requestMoves();

  updateGame(style12.index(), last_move, style12.position);
}

void ICSEntity::notifyPool(const PoolInfo& pi) {
  if(pi.m_game_num != m_game_number)
    return;

  if(m_game->containsIndex(pi.m_pos_index)) {
    AbstractPosition::Ptr p = m_game->position(pi.m_pos_index);
    p->setPool(pi.m_pool);
    m_game->insert(m_game->move(pi.m_pos_index), p, pi.m_pos_index );
  }
}

void ICSEntity::notifyMoveList(int num, AbstractPosition::Ptr pos, const PGN& pgn) {
  if (m_game_number != num) return;

  if(pos->variant() != m_variant->name()) {
    std::cout << "Mismatch for move list, this variant is \"" << m_variant->name() << "\",\n"
                " got move list for \"" << pos->variant() << "\"" << std::endl;
    return;
  }

  Game g;
  g.reset(pos);
  g.load(pgn);

  for(int i=0;i<=g.lastMainlineIndex().num_moves;i++) {
    m_game->insert(g.move(i), g.position(i), i);
  }
  //m_game->load(pos, pgn);
}

bool ICSEntity::canDetach() const {
  // TODO: ask whether abort or resign...
  return true;
}


bool ICSEntity::attach() {
  // an ICS player can attach only if no move for that side has been made
  //std::cout << "[ICS attach] index = " << m_game->lastMainlineIndex() << std::endl;
  if (m_game->lastMainlineIndex() == 0) return true;
  if (m_game->lastMainlineIndex() == 1 && m_side == 1) {
    // white already played, inform black
    m_dispatcher.move(m_game->move(0), m_game->position(0));
    return true;
  }
  return false;
}

void ICSEntity::notifyMove(AbstractMove::Ptr move, AbstractPosition::Ptr ref) {
  //Use coordinates bacause FICS is stupid and would consider bxc6 ambiguous (Bxc6)
  m_connection->sendText(move->toString(ref));
  //m_connection->sendText(move->SAN(ref));
}

void ICSEntity::requestMoves() {
  m_connection->sendText(QString("moves %1").arg(m_game_number));
}


ObservingEntity::ObservingEntity(VariantInfo* variant, const shared_ptr<Game>& game,
                   int gameNumber,const shared_ptr<ICSConnection>& connection, AgentGroup* group)
: ICSEntity(variant, game, -1, gameNumber, connection, group)
, m_attached(true) { }

void ObservingEntity::detach() {
  m_attached = false;
}

ObservingEntity::~ObservingEntity() {
  if (m_attached)
    m_connection->sendText(QString("unobs %1").arg(m_game_number));
}
