/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@gmail.com>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef ENGINEENTITY_H
#define ENGINEENTITY_H

#include "agent.h"
#include "agentgroup.h"
#include "enginenotifier.h"
#include "entity.h"
#include "index.h"
#include "tagua.h"
#include "fwd.h"

class Engine;

class EngineEntity : public Entity
                   , public Agent
                   , public EngineNotifier {
  VariantPtr m_variant;
  int m_side;
  Index m_last_index;
  bool m_playing;
  boost::shared_ptr<Engine> m_engine;
  AgentGroupDispatcher m_dispatcher;

  void checkPlaying();
public:
  boost::shared_ptr<Engine> engine() { return m_engine; }

  EngineEntity(const VariantPtr& variant, const boost::shared_ptr<Game>&, int side,
              const boost::shared_ptr<Engine>& engine, AgentGroup* group);
  void setup();

  virtual void notifyEngineMove(const QString&);

  virtual bool canDetach() const { return true; }

  virtual void notifyClockUpdate(int, int) { }
  virtual void notifyMove(const Index& index);
  virtual void notifyBack() { }
  virtual void notifyForward() { }
  virtual void notifyGotoFirst() { }
  virtual void notifyGotoLast() { }
};

#endif // ENGINEENTITY_H
