/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@sns.it>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>
            
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef PATHINFO_H
#define PATHINFO_H

class PathInfo {
public:
  enum Direction {
    Undefined,
    Horizontal,
    Vertical,
    Diagonal1,
    Diagonal2
  };
private:
  Direction m_direction;
  bool m_clear;
public:

  PathInfo(Direction direction, bool clear);

  bool parallel() const { return m_direction == Horizontal || m_direction == Vertical; }
  bool diagonal() const { return m_direction == Diagonal1 || m_direction == Diagonal2; }
  bool clear() const { return m_clear; }
  bool valid() const { return m_direction != Undefined; }
};

#endif // PATHINFO_H
