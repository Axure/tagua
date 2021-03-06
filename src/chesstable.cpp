/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@gmail.com>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include <QLayout>
#include <QSplitter>
#include <QMouseEvent>
#include <KDebug>
#include "chesstable.h"
#include "game.h"
#include "gameinfo.h"
#include "connection.h"
#include "piecepool.h"
#include "clock.h"
#include "mastersettings.h"
#include "movelist_table.h"
#include "infodisplay.h"

using namespace boost;

ChessTable::ChessTable(QWidget* parent)
: KGameCanvasWidget(parent)
, m_wallpaper(NULL)
, m_current(NULL)
, m_mousegrab(NULL)
, m_need_reload(false) {

  setMouseTracking(true);

  // create m_board
  m_board = new Board(m_anim_settings, this);
  m_board->show();

  // create move list
  m_movelist = new MoveList::Table;

  // create clocks
  for(int i=0;i<2;i++) {
    m_clocks[i] = new Clock(i, this);
    m_clocks[i]->show();
  }
  m_clocks[0]->activate(0);

  // create info display
  m_info = new InfoDisplay(this);
//  m_info->show();

  // create pools
  for(int i=0;i<2;i++) {
    m_pools[i] = new PiecePool(i, m_board, m_anim_settings, this);
    m_pools[i]->show();
  }

  m_board->raise();
  settingsChanged();
}

ChessTable::~ChessTable() {
  for(int i=0;i<2;i++)
  delete m_clocks[i];
  delete m_movelist;
  delete m_board;
  for(int i=0;i<2;i++)
    delete m_pools[i];
  delete m_info;
}

void ChessTable::renderWallpaper() {
  if (!m_background_pixmap.isNull()) {
    QPixmap bg = m_background_pixmap;
    // scale & crop background
    double ratio_x = (double) width() / m_background_pixmap.width();
    double ratio_y = (double) height() / m_background_pixmap.height();
    if (ratio_x > ratio_y)
      bg = bg.scaledToWidth(width(), Qt::SmoothTransformation);
    else
      bg = bg.scaledToHeight(height(), Qt::SmoothTransformation);
    QSize size(width(), height());
    QImage res(size, QImage::Format_ARGB32_Premultiplied);
    {
      QPoint pos(
        (bg.width() - width()) / 2, 
        (bg.height() - height()) / 2);
      kDebug() << "img size" << bg.size();
      kDebug() << "size" << size;
      kDebug() << "pos" << pos;
      QPainter p(&res);
      p.drawPixmap(QPoint(0,0), bg, QRectF(pos, size));
    }
  
    bg = QPixmap::fromImage(res);
    if (m_wallpaper) {
      m_wallpaper->setPixmap(bg);
    }
    else {
      delete m_wallpaper;
      m_wallpaper = new KGameCanvasPixmap(QPixmap::fromImage(res), this);
      m_wallpaper->lower();
      m_wallpaper->show();
    }
  }
  else {
    delete m_wallpaper;
    m_wallpaper = 0;
  }
}

void ChessTable::settingsChanged() {
  m_anim_settings.reload();

  m_board->settingsChanged();
  for(int i=0;i<2;i++)
    m_clocks[i]->settingsChanged();
  m_info->settingsChanged();
  for(int i=0;i<2;i++)
    m_pools[i]->settingsChanged();
    
  m_background_pixmap = m_board->controlsLoader()->getStaticValue<QPixmap>("wallpaper", 0, true);
  renderWallpaper();

  /* redo the layout, forcing reload */
  if(isVisible())
    layout(true);
  else
    m_need_reload = true;
}

ClickableCanvas* ChessTable::eventItemAt(QPoint pos) {
  if (m_board->boardRect().contains(pos))
    return m_board;

  for (int i=0; i<2; i++)
  if (m_pools[i]->boardRect().contains(pos))
    return m_pools[i];

  for (int i=0; i<2; i++)
  if (m_clocks[i]->rect().contains(pos))
    return m_clocks[i];

  return NULL;
}

void ChessTable::setEntity(const boost::shared_ptr<UserEntity>& entity) {
  m_board->setEntity(entity);
}

void ChessTable::layout(bool force_reload) {
  force_reload |= m_need_reload;
  m_need_reload = false;

  renderWallpaper();

  ::LuaApi::LuaValueMap params;
  params["width"] = width();
  params["height"] = height();
  params["grid_size"] = QPointF(m_board->gridSize());

  ::LuaApi::LuaValueMap lvals = m_board->controlsLoader()->getStaticValue< ::LuaApi::LuaValueMap>("layout", &params);

#if 0
  for(::LuaApi::LuaValueMap::iterator it = lvals.begin(); it != lvals.end(); ++it)
  if(double* val = boost::get<double>(&it.value()))
    kDebug() << "lvals[" << it.key() << "] = " << *val;
  else if(QPointF* val = boost::get<QPointF>(&it.value()))
    kDebug() << "lvals[" << it.key() << "] = Point(" << val->x() << "," << val->y() << ")";
  else if(QRectF* val = boost::get<QRectF>(&it.value()))
    kDebug() << "lvals[" << it.key() << "] = Rect(" << val->x() << "," << val->y()
                                   << "," << val->width() << "," << val->height() << ")";
#endif

#define GET_INT(name)                                    \
  int name = 0;                                          \
  {::LuaApi::LuaValueMap::iterator it = lvals.find(#name);\
  if(double* val = (it==lvals.end()) ? 0 : boost::get<double>(&lvals[#name]) )  \
    name = (int)*val;                                    \
  else                                                   \
    kError() << "Theme error:" << #name << "should be set to a number in the layout";}

#define GET_POINT(name)                                  \
  QPoint name;                                           \
  {::LuaApi::LuaValueMap::iterator it = lvals.find(#name);\
  if(QPointF* val = (it==lvals.end()) ? 0 : boost::get<QPointF>(&lvals[#name]) )  \
    name = val->toPoint();                               \
  else                                                   \
    kError() << "Theme error:" << #name << "should be set to a point in the layout";}

  GET_POINT(board_position);
  GET_INT(square_size);
  GET_INT(border_size);
  GET_INT(border_text_near);
  GET_INT(border_text_far);
  GET_POINT(clock0_position);
  GET_POINT(clock1_position);
  GET_INT(clock_size);
  GET_POINT(pool0_position);
  GET_POINT(pool1_position);
  GET_INT(pool_piece_size);
  GET_INT(pool_width);

  m_board->moveTo(board_position.x(), board_position.y());
  m_board->onResize( square_size, border_size, border_text_near, border_text_far, force_reload);

  int x = !m_board->flipped();

  m_clocks[x]->resize(clock_size);
  m_clocks[x]->moveTo(clock0_position.x(), clock0_position.y());
//   kDebug() << "moving clock " << x << " to " << clock0_position.y();

  m_clocks[!x]->resize(clock_size);
  m_clocks[!x]->moveTo(clock1_position.x(), clock1_position.y());
//   kDebug() << "moving clock " << !x << " to " << clock1_position.y();

  m_pools[x]->m_flipped = false;
  m_pools[x]->onResize(pool_piece_size, force_reload);
  m_pools[x]->moveTo(pool0_position.x(), pool0_position.y());
  m_pools[x]->setGridWidth(pool_width);

  m_pools[!x]->m_flipped = true;
  m_pools[!x]->onResize(pool_piece_size, force_reload);
  m_pools[!x]->moveTo(pool1_position.x(), pool1_position.y());
  m_pools[!x]->setGridWidth(pool_width);
}

void ChessTable::resizeEvent(QResizeEvent* /*e*/) {
  layout();
}

void ChessTable::mouseReleaseEvent(QMouseEvent* e) {

  if(m_mousegrab) {
    m_mousegrab->onMouseRelease(e->pos() - m_mousegrab->pos(), e->button() );
    if(!e->buttons()) {
      m_mousegrab = NULL;

      ClickableCanvas* cb = eventItemAt(e->pos());
      if(cb != m_current) {
        if(m_current)
          m_current->onMouseLeave();
        if(cb) {
          cb->onMouseEnter();
          cb->onMouseMove(e->pos() - cb->pos(), 0);
        }
        m_current = cb;
      }
    }
    return;
  }
}

void ChessTable::mousePressEvent(QMouseEvent* e) {
  if(m_mousegrab) {
    m_mousegrab->onMousePress(e->pos() - m_mousegrab->pos(), e->button() );
    return;
  }

  ClickableCanvas* cb = eventItemAt(e->pos());
  if(cb != m_current) {
    if(m_current)
      m_current->onMouseLeave();
    if(cb)
      cb->onMouseEnter();
    m_current = cb;
  }
  if(cb) {
    cb->onMousePress(e->pos() - cb->pos(), e->button() );
    m_mousegrab = cb;
  }
}

void ChessTable::mouseMoveEvent(QMouseEvent* e) {
  if(m_mousegrab) {
    m_mousegrab->onMouseMove(e->pos() - m_mousegrab->pos(), e->button() );
    return;
  }

  ClickableCanvas* cb = eventItemAt(e->pos());
  if(cb != m_current) {
    if(m_current)
      m_current->onMouseLeave();
    if(cb)
      cb->onMouseEnter();
    m_current = cb;
  }
  if(cb)
    cb->onMouseMove(e->pos() - cb->pos(), e->button() );
}

void ChessTable::enterEvent(QEvent*) { }

void ChessTable::leaveEvent(QEvent*) {
  if(m_current)
    m_current->onMouseLeave();
  m_current = NULL;
}

void ChessTable::flip() {
  m_board->flip();

  int delta = qAbs(m_pools[0]->pos().y() - m_pools[1]->pos().y());
  for(int i=0;i<2;i++)
    m_pools[i]->flipAndMoveBy( QPoint(0, delta) );
    
  // flip clocks
  QPoint pos = m_clocks[0]->pos();
  m_clocks[0]->moveTo(m_clocks[1]->pos());
  m_clocks[1]->moveTo(pos);
}

void ChessTable::flip(bool flipped) {
  if(m_board->flipped() != flipped)
    flip();
}

void ChessTable::changeClock(int color) {
  if(m_clocks[0]->running() || m_clocks[1]->running())
  for(int i=0;i<2;i++) {
    if ( (i == color) != m_clocks[i]->running() )
    if( i==color )
      m_clocks[i]->start();
    else
      m_clocks[i]->stop();
  }
}

void ChessTable::updateTurn(int color) {
  for(int i=0; i<2; i++)
    m_clocks[i]->activate(color == i);
}

void ChessTable::stopClocks() {
  for(int i=0; i<2; i++)
    m_clocks[i]->stop();
}

void ChessTable::updateTime(int white, int black) {
  m_clocks[0]->setTime(white);
  m_clocks[1]->setTime(black);
}

void ChessTable::resetClock() {
  stopClocks();
  updateTime(0, 0);
  for(int i=0; i<2; i++)
    m_clocks[i]->setPlayer(Player());
}

void ChessTable::setPlayers(const Player& white, const Player& black) {
  m_clocks[0]->setPlayer(white);
  m_clocks[1]->setPlayer(black);
}

void ChessTable::run() {
  for(int i=0;i<2;i++)
  if(m_clocks[i]->active() && !m_clocks[i]->running())
    m_clocks[i]->start();
}

void ChessTable::displayMessage(const QString& msg) {
  kDebug() << msg;
  message(msg);
}

const AnimationSettings& ChessTable::animationSettings() const {
  return m_anim_settings;
}

