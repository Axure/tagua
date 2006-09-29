/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@sns.it>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>
            
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef MAINANIMATION_H
#define MAINANIMATION_H

#include <QObject>
#include <QTime>
#include <QTimer>
#include "animation.h"
#include <boost/shared_ptr.hpp>

/**
  * @a MainAnimation is a persistent animation used to activate
  * all chessboard animations.
  */
class MainAnimation : public QObject, protected AnimationGroup {
Q_OBJECT
  QTime m_time;
  QTimer m_timer;
  int    m_translate;
  double m_speed;

public:
  MainAnimation(double speed = 1.0);

  /**
    * Activate the animation @a a.
    */
  void addAnimation(const boost::shared_ptr<Animation>& a);

  /**
    * Stop all animations.
    */
  void stop();

  /**
    * Sets the overall speed factor (default is 1.0).
    * This function is implemented to avoid time gaps.
    */
  void setSpeed(double speed);

  /**
    * Sets the frame delay.
    */
  void setDelay(int delay);

protected slots:
  void tick();
};

#endif // MAINANIMATION_H
