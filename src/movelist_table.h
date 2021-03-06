/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@gmail.com>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef MOVELIST_TABLE_H
#define MOVELIST_TABLE_H

#include <QWidget>
#include "movelist_widget.h"

class QAction;
class QScrollArea;
class ThemeInfo;

namespace MoveList {
  class Widget;
  class Textual;

  /**
    * @class Table <movelist_table.h>
    * @brief The widget that holds the move list in a scrolled view.
    *
    * This class is a widget that contains a MoveList::Widget and a few
    * convenience buttons for undo and redo.
    */
  class Table : public QWidget {
  Q_OBJECT
  public:
    /** the scroll area that contains the move list */
    QScrollArea *m_scroll_area;

    /** the move list widget */
    Widget *m_movelist;

    /** the move list textual widget */
    Textual *m_movelist_textual;

    Table(QWidget* p = NULL);

    void settingsChanged();

    void setLoaderTheme(const ThemeInfo& theme);

    /** Sets the move comment at the given index */
    void setComment(const Index& index, const QString& comment);

    /** Sets the comment at the given index before the give subvariation */
    void setVComment(const Index& index, int v, const QString& comment, bool confirm_promotion = false);

    /** Sets the move at the given index */
    void setMove(const Index& index, int turn, const DecoratedMove& move,
                                    const QString& comment = QString(), bool confirm_promotion = false);

    /** Sets the move at the given index */
    void setMove(const Index& index, int turn, const QString& move,
                                    const QString& comment = QString(), bool confirm_promotion = false);

    /** Removes the given index and all those that come after */
    void remove(const Index& index, bool confirm_promotion = false);

    /** Promotes the given variation in the main line */
    void promoteVariation(const Index& ix, int v);

    void reset();

    Notifier* getNotifier();

    void setNotifier(Notifier* n, bool detach_prev=true);

    int layoutStyle();

    void setLayoutStyle(int x);

    /** Sets the currently selected index */
    void select(const Index& index, bool confirm_promotion = false);

  private Q_SLOTS:
    void onUndo();
    void onRedo();
  };
} //end namespace MoveList

#endif //MOVELIST_TABLE_H
