/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@gmail.com>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>
            
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "random.h"

Random* Random::m_instance = 0;

Random::Random() { }

Random::IntegerGenerator Random::rand(int min, int max) {
  return IntegerGenerator(&m_generator, IntegerDistribution(min, max));
}

Random::RealGenerator Random::rand(float min, float max) {
  return RealGenerator(&m_generator, RealDistribution(min, max));
}

Random::RealGenerator Random::rand(double min, double max) {
  return rand(static_cast<float>(min),
              static_cast<float>(max));
}

Random& Random::instance() {
  if (!m_instance) {
    m_instance = new Random;
  }
  return *m_instance;
}

