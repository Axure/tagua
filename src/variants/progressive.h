/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@sns.it>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>
            
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef PROGRESSIVE_H
#define PROGRESSIVE_H

class VariantInfo;

class ProgressiveVariant {
  static VariantInfo* static_variant_info;
public:
  static VariantInfo* info();
};

#endif // PROGRESSIVE_H
