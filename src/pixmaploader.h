/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@sns.it>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef PIXMAPLOADER_H
#define PIXMAPLOADER_H

#include <QPixmap>
#include <QString>
#include "loader/theme.h"

/**
  * @class PixmapLoader <pixmaploader.h>
  * @brief The simple pixmap loading facility.
  *
  * This is the utility class used by the interface to load pixmap from resource ids.
  * It acts simply as a wrapper around PixmapLoader::ThemeLoader, making it possible to
  * use it in a simple and afficient way.
  *
  * Note that pixmaps will not be all of the specified size, the size is only the base size.
  */
class PixmapLoader {
private:
  class ThemeLoader;
  typedef std::map<QString, ThemeLoader*> ThemeLoadersCache;

  /** static cache of the loaders, there should be only one for each (theme,variant) pair  */
  static ThemeLoadersCache s_loaders;

  /** the current loader, or NULL */
  ThemeLoader *m_loader;

  /** the current size */
  int m_size;

  /** the current theme */
  QString m_base;

  /** internal, clears references to the currently used loader, if any. */
  void flush();

  /** internal, gets or creates a loader good for the current
      (theme,variant) pair and refs the size */
  void initialize();

public:
  /** constructor */
  PixmapLoader();
  ~PixmapLoader();

  /** set the theme path */
  void setBasePath(const QString& base);

  /** set the base size of the pixmaps. Note that returned pixmaps's size can be different.
    * For instance, if the size is 100 the "background" generated by the
    * chess variant will be 200x200 (and it will be tiled on the Board)
    */
  void setSize(int s);

  /** looks up a string id (for instance a predefined id, like "background" or "highlighting") */
  QPixmap operator()(const QString& id);

  /** looks up a string id (for instance a predefined id, like "background" or "highlighting") */
  Loader::PixmapOrMap getPixmapMap(const QString& id);

  /** looks up a font + char */
  Loader::Glyph getGlyph(const QString& id);
};

#endif // PIXMAPLOADER_H
