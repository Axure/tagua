/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@sns.it>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include <iostream>
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>

#include "settings.h"
#include "board.h"
#include "piecesprite.h"
#include "animation.h"
#include "boardsprite.h"
#include "pointconverter.h"
#include "entities/userentity.h"
#include "mainanimation.h"
#include "premove.h"

using namespace boost;

/** inherit instead of typedef to ease forward declaration :) */
class BoardTags : public std::map<QString, std::map<Point, boost::shared_ptr<Canvas::Pixmap> > > {
};

Board::Board(Canvas::Abstract* parent)
: PieceGroup(parent)
, m_sprites(0,0)
, m_hinting_pos(Point::invalid())
, selection(Point::invalid())
, lastSelection(Point::invalid()) {

  m_tags = BoardTagsPtr(new BoardTags);

  m_canvas_background = new Canvas::TiledPixmap(this);
  m_canvas_background->lower();
  m_canvas_background->show();

  m_pieces_group = new Canvas::Group(this);
  m_pieces_group->show();

  mySettingsChanged();
}

Board::~Board() {
}

void Board::mySettingsChanged() {

  m_show_border = (settings["BoardBorderShow"]|=true).value<bool>();
  m_border_color = (settings["BoardBorderColor"]|=QColor(Qt::white)).value<QColor>();
  m_border_text_color = (settings["BoardBorderTextColor"]|=QColor(Qt::black)).value<QColor>();
  m_border_font = (settings["BoardBorderFont"]|=QApplication::font()).value<QFont>();

  recreateBorder();
}

void Board::settingsChanged() {
  PieceGroup::settingsChanged();
  mySettingsChanged();
}

boost::shared_ptr<Canvas::Pixmap> Board::addTag(const QString& name, Point pt, bool over) {
  if(!m_sprites.valid(pt))
    return boost::shared_ptr<Canvas::Pixmap>();

  QPixmap p = m_tags_loader(name);
  boost::shared_ptr<Canvas::Pixmap> item =
      boost::shared_ptr<Canvas::Pixmap>(new Canvas::Pixmap(p, this));
  item->moveTo(converter()->toReal(pt));
  if(over)
    item->stackOver(m_pieces_group);
  else
    item->stackUnder(m_pieces_group);
  item->show();

  (*m_tags)[name][pt] = item;
  return item;
}

void Board::clearTags(const QString& name) {
  m_tags->erase(name);
}

void Board::clearTags() {
  m_tags->clear();
}

void Board::setTags(const QString& name, Point p1, Point p2, Point p3,
                                          Point p4, Point p5, Point p6 ) {
  //TODO: maybe this could be optimized a bit
  clearTags(name);
  addTag(name, p1);
  addTag(name, p2);
  addTag(name, p3);
  addTag(name, p4);
  addTag(name, p5);
  addTag(name, p6);
}

void Board::recreateBorder() {
  m_border_margins.clear();
  m_border_items.clear();

  if(!m_show_border || m_border_coords.size() == 0) {
    m_border_size = 0;
    return;
  }

  QFontMetrics fm(m_border_font);
  m_border_size = std::max( fm.height(), fm.boundingRect("H").width() );
  m_border_asc = fm.ascent() + (m_border_size-fm.height())/2;

  for(int w = 0; w<2; w++)
  for(int i=0;i<4;i++) {
    Canvas::Rectangle *item = new Canvas::Rectangle(
          w ? m_border_text_color : m_border_color, QSize(), this);
    item->show();
    m_border_margins.push_back( boost::shared_ptr<Canvas::Rectangle>( item ));
  }

  for(int w = 0; w<2; w++)
  for(int i = 0;i<m_sprites.getSize().x;i++) {
    QString l = m_border_coords.size()>i ? m_border_coords[i] : QString();
    Canvas::Item *item = new Canvas::Text( l, m_border_text_color, m_border_font,
                                Canvas::Text::HCenter, Canvas::Text::VBaseline, this);
    item->show();
    m_border_items.push_back( boost::shared_ptr<Canvas::Item>( item ));
  }

  for(int w = 0; w<2; w++)
  for(int i = 0;i<m_sprites.getSize().y;i++) {
    QString n = m_border_coords.size()>i+m_sprites.getSize().x
                        ? m_border_coords[i+m_sprites.getSize().x] : QString();
    Canvas::Item *item = new Canvas::Text( n, m_border_text_color, m_border_font,
                Canvas::Text::HCenter, Canvas::Text::VBaseline, this);
    item->show();
    m_border_items.push_back( boost::shared_ptr<Canvas::Item>( item ));
  }

  m_pieces_group->raise();

  updateBorder();
}

void Board::updateBorder() {
  if(!m_show_border)
    return;

  int at = 0;
  for(int w = 0; w<2; w++)
  for(int i = 0;i<m_sprites.getSize().x;i++) {
    int x = (m_flipped ? (m_sprites.getSize().x-1-i):i)*m_square_size+m_square_size/2;
    int y = m_border_asc+(w?0:m_border_size+m_square_size*m_sprites.getSize().y)-m_border_size;
    m_border_items[at++]->moveTo(x, y);
  }

  for(int w = 0; w<2; w++)
  for(int i = 0;i<m_sprites.getSize().y;i++) {
    int x = -m_border_size/2+(w?0:m_border_size+m_square_size*m_sprites.getSize().x);
    int y = (!m_flipped ? (m_sprites.getSize().y-1-i):i)*m_square_size
                        +m_square_size/2-m_border_size/2+m_border_asc;
    m_border_items[at++]->moveTo(x, y);
  }

  m_border_margins[0]->moveTo(-m_border_size,-m_border_size);
  m_border_margins[0]->setSize(QSize(2*m_border_size+m_square_size*m_sprites.getSize().x,
                                                              m_border_size));
  m_border_margins[4]->moveTo(-m_border_size-1,-m_border_size-1);
  m_border_margins[4]->setSize(QSize(2*m_border_size+m_square_size*m_sprites.getSize().x+2, 1));

  m_border_margins[1]->moveTo(-m_border_size,m_square_size*m_sprites.getSize().y);
  m_border_margins[1]->setSize(QSize(2*m_border_size+m_square_size*m_sprites.getSize().x,
                                                              m_border_size));
  m_border_margins[5]->moveTo(-m_border_size-1,m_square_size*m_sprites.getSize().y+m_border_size);
  m_border_margins[5]->setSize(QSize(2*m_border_size+m_square_size*m_sprites.getSize().x+2, 1));

  m_border_margins[2]->moveTo(-m_border_size,0);
  m_border_margins[2]->setSize(QSize(m_border_size, m_square_size*m_sprites.getSize().y));
  m_border_margins[6]->moveTo(-m_border_size-1,-m_border_size);
  m_border_margins[6]->setSize(QSize(1, m_square_size*m_sprites.getSize().y+2*m_border_size));

  m_border_margins[3]->moveTo(m_square_size*m_sprites.getSize().x, 0);
  m_border_margins[3]->setSize(QSize(m_border_size, m_square_size*m_sprites.getSize().y));
  m_border_margins[7]->moveTo(m_square_size*m_sprites.getSize().x+m_border_size, -m_border_size);
  m_border_margins[7]->setSize(QSize(1, m_square_size*m_sprites.getSize().y+2*m_border_size));
}

void Board::createGrid(Point p, const QStringList& border_coords) {
  m_border_coords = border_coords;
  m_sprites = PieceGrid(p.x,p.y);
  recreateBorder();
}

QRect Board::computeRect(Point p) const {
  QPoint realPoint = converter()->toReal(p);
  return squareRect(realPoint.x(), realPoint.y());
}

QRect Board::squareRect(int x, int y) const {
  return QRect(x, y, m_square_size, m_square_size);
}

QRegion Board::computeRegion(Point p) const {
  return QRegion(computeRect(p));
}

// selection
void Board::setSelection(const Point& p) {
  lastSelection = selection;
  selection = p;
  setTags("selection", p);
}

void Board::cancelSelection() {
  lastSelection = selection;
  selection = Point::invalid();
  clearTags("selection");
}

// premove

void Board::setPremove(const NormalUserMove& premove) {
  m_premove_from = premove.from;
  m_premove_to = premove.to;
  setTags("premove", m_premove_from, m_premove_to);
}

void Board::setPremove(const DropUserMove& premove) {
  m_premove_from = Point::invalid();
  m_premove_to = premove.m_to;
  setTags("premove", m_premove_to);
}

void Board::setPremove(const Premove& premove) {
  setPremove(premove.toUserMove());
}

void Board::cancelPremove() {
  m_premove_from = Point::invalid();
  m_premove_to = Point::invalid();
  clearTags("premove");
}

void Board::updateSprites() {
  // adjust piece positions
  for (Point i = m_sprites.first(); i <= m_sprites.last(); i = m_sprites.next(i)) {
    boost::shared_ptr<PieceSprite> p = m_sprites[i].sprite();

    if (p) {
      // drawing sprite
      p->setPixmap( m_loader( m_sprites[i].name() ) );
      adjustSprite(i);
    }
  }
}

void Board::updateTags() {
  for(BoardTags::iterator tit = m_tags->begin(); tit != m_tags->end(); ++tit)
  for(std::map<Point, boost::shared_ptr<Canvas::Pixmap> >::iterator pt =
                          tit->second.begin(); pt != tit->second.end(); ++pt) {
    pt->second->moveTo(converter()->toReal(pt->first));
    pt->second->setPixmap(m_tags_loader(tit->first));
  }
}

bool Board::doMove(const NormalUserMove& m) {
  if (m_entity.lock()->oneClickMoves() || m_entity.lock()->validTurn(m.from) == Entity::Moving) {
    AbstractMove::Ptr mv = m_entity.lock()->testMove(m);
    if (mv) {
      m_entity.lock()->executeMove(mv);
      return true;
    }
  }

  std::cout << "invalid move" << std::endl;
  emit error(InvalidMove);

  return false;
}


void Board::onResize(int new_size, bool force_reload) {
  if(m_square_size == new_size && !force_reload)
    return;

  PieceGroup::onResize(new_size);

  // update the size of the tag loader
  m_tags_loader.setSize(m_square_size);

  // update canvas background
  m_canvas_background->setSize(QSize(boardSizeX(), boardSizeY()));
  m_canvas_background->setPixmap(m_tags_loader("background"));

  // update the sprites
  updateSprites();

  // update piece tags
  updateTags();

  // update border
  updateBorder();
}

void Board::onMousePress(const QPoint& pos, int button) {
  Point point = converter()->toLogical(pos);
  if (!m_sprites.valid(point))
      return;

  //BEGIN left click

  if (button == Qt::LeftButton) {
    if (m_entity.lock()->oneClickMoves()) {
      NormalUserMove m(Point::invalid(), point);
      m_entity.lock()->setPromotion(m);
      doMove(m);
    }
    else {
      shared_ptr<PieceSprite> piece = m_sprites[point].sprite();

      if (piece && m_entity.lock()->movable(point)) {
          cancelSelection();
          m_drag_info = DragInfoPtr(new DragInfo(point, piece,
                            m_entity.lock()->validTurn(point)) );
          piece->raise();
      }

      // if selection is valid, (pre)move to point
      else if (selection != Point::invalid()) {
          piece = m_sprites[selection].sprite();
          Q_ASSERT(piece);
          NormalUserMove m(selection, point);
          m_entity.lock()->setPromotion(m);

          switch(m_entity.lock()->validTurn(selection)) {

            case UserEntity::Moving:
              doMove(m);
              cancelSelection();
              break;

            case UserEntity::Premoving:
              if (m_entity.lock()->testPremove(m)) {
                m_entity.lock()->addPremove(m);
                setPremove(m);
                cancelSelection();
              }
              break;

            default:
              break;
          }
      }
    }
  }

  //END left click

  //BEGIN right click

  else if (button == Qt::RightButton) {
    cancelSelection();
    if (point == m_premove_from || point == m_premove_to)
      cancelPremove();
    m_entity.lock()->handleRightClick(point);
  }

  //END right click
}

void Board::onMouseRelease(const QPoint& pos, int button) {
  Point point = converter()->toLogical(pos);

  //BEGIN left click

  if (button == Qt::LeftButton) {

    if (m_drag_info) {
//      Q_ASSERT(m_drag_info->piece);
      Q_ASSERT(m_drag_info->sprite);
      bool moved = false;

      // remove valid move highlighting
      clearTags("validmove");

      // toggle selection if the piece didn't move
      if (m_drag_info->from == point) {
        if (lastSelection == point)
          cancelSelection();
        else
          setSelection(point);
      }

      else  {
        NormalUserMove m(m_drag_info->from, point, true);
        if (!m_sprites.valid(point))
          m.to = Point::invalid();

        m_entity.lock()->setPromotion(m);

        switch(m_entity.lock()->validTurn(m_drag_info->from)) {

          case UserEntity::Moving:
            if (doMove(m))
              moved = true;
            break;

          case UserEntity::Premoving:
            if (m_entity.lock()->testPremove(m)) {
              m_entity.lock()->addPremove(m);
              setPremove(m);
            }
            break;

          default:
            break;
        }
      }

      shared_ptr<PieceSprite> s = m_sprites[m_drag_info->from].sprite();
      if (!moved && s && s->pos() != converter()->toReal(m_drag_info->from)) {
        Q_ASSERT(s);
        QPoint real = converter()->toReal(m_drag_info->from);
        if(!m_anim_movement)
          enqueue(shared_ptr<Animation>(new InstantAnimation(s, real)));
        else if (point == m_drag_info->from)
          enqueue(shared_ptr<Animation>(new MovementAnimation(s, real)));
        else
          enqueue(shared_ptr<Animation>(new TeleportAnimation(s, s->pos(), real)));
      }

      m_drag_info = DragInfoPtr();
    }
  }

  //END left button
}

void Board::onMouseMove(const QPoint& pos, int /*button*/) {
  Point point = converter()->toLogical(pos);

  if (m_drag_info) {
    Q_ASSERT(m_drag_info->sprite);
    m_drag_info->sprite->moveTo(pos - QPoint(m_square_size / 2, m_square_size / 2) );

    // highlight valid moves
    NormalUserMove move(m_drag_info->from,  point);
    bool valid = m_sprites.valid(point);
    if (valid) {
      UserEntity::Action action = m_entity.lock()->validTurn(m_drag_info->from);
      if (action == UserEntity::Moving)
        valid = m_entity.lock()->testMove(move);
    }

    if (valid)
      setTags("validmove", point);
    else
      clearTags("validmove");
  }
  else if (m_entity.lock()->oneClickMoves()) {
    if(point == m_hinting_pos)
      return;

    AbstractPiece::Ptr hint;

    if (m_sprites.valid(point)) {
      if (AbstractMove::Ptr move = m_entity.lock()->testMove(
                              NormalUserMove(Point::invalid(), point))) {
        // set move hint
        hint = m_entity.lock()->moveHint(move);
      }
    }

    updateHinting(point, hint);
  }
}

void Board::onPositionChanged() {
  if (m_entity.lock() && m_entity.lock()->oneClickMoves() && m_sprites.valid(m_hinting_pos)) {
    AbstractPiece::Ptr hint;

    if (AbstractMove::Ptr move = m_entity.lock()->testMove(
                            NormalUserMove(Point::invalid(), m_hinting_pos)) ) {
      // set move hint
      hint = m_entity.lock()->moveHint(move);
    }

    updateHinting(m_hinting_pos, hint);
  }
}

void Board::onMouseLeave() {
  updateHinting(Point::invalid(), AbstractPiece::Ptr());
}

void Board::updateHinting(Point pt, AbstractPiece::Ptr piece) {
  if(!m_sprites.valid(pt))
    piece = AbstractPiece::Ptr();

  if(!piece || !m_sprites.valid(pt)) {
    if(m_hinting.sprite()) {
      if(m_anim_fade)
        enqueue( boost::shared_ptr<Animation>(new FadeAnimation(m_hinting.sprite(),
                                                        m_hinting.sprite()->pos(), 160, 0)) );
      else
        enqueue( boost::shared_ptr<Animation>(new CaptureAnimation(m_hinting.sprite())) );
    }

    m_hinting_pos = Point::invalid();
    m_hinting = Element();
  }
  else {
    if(pt == m_hinting_pos) {
      if(!piece->equals(m_hinting.piece())) {
        m_hinting = Element(piece, m_hinting.sprite());
        m_hinting.sprite()->setPixmap(m_loader(piece->name()));
      }
    }
    else {
      if(m_hinting.sprite()) {
        if(m_anim_fade)
          enqueue( boost::shared_ptr<Animation>(new FadeAnimation(m_hinting.sprite(),
                                                          m_hinting.sprite()->pos(), 160, 0)) );
        else
          enqueue( boost::shared_ptr<Animation>(new CaptureAnimation(m_hinting.sprite())) );
      }

      QPixmap pix = m_loader(piece->name());
      boost::shared_ptr<PieceSprite> sprite = createSprite(pix, pt);
      sprite->setOpacity(160);
      sprite->raise();
      sprite->show();

      m_hinting_pos = pt;
      m_hinting = Element(piece, sprite);

      /*if(m_anim_fade)
        enqueue( boost::shared_ptr<Animation>(new FadeAnimation(m_hinting.sprite(),
                                                        m_hinting.sprite()->pos(), 0, 160)) );
      else {
        m_hinting.sprite()->setOpacity(160);
        enqueue(boost::shared_ptr<Animation>(new DropAnimation(m_hinting.sprite())) );
      }*/
    }
  }
}

void Board::reset() {
  clearTags();
  cancelSelection();
  cancelPremove();
  m_main_animation->stop();
}

void Board::flip(bool flipped)
{
  if (m_flipped != flipped) {
    m_flipped = flipped;

    // update sprite positions
    for (Point i = m_sprites.first(); i <= m_sprites.last(); i = m_sprites.next(i))
    if (m_sprites[i].sprite())
      animatePiece(m_sprites[i].sprite(), i, 1.0);

    updateTags();
    updateBorder();
  }
}

void Board::draggingOn(AbstractPiece::Ptr piece, const QPoint& point) {
  Point to = converter()->toLogical(point);

  if (m_sprites.valid(to))
  switch(m_entity.lock()->validTurn(piece->color())) {
    case UserEntity::Moving: {
      DropUserMove m(piece, to);
      AbstractMove::Ptr mv = m_entity.lock()->testMove(m);
      if (mv) {
        setTags("validmove", to);
        return;
      }
      break;
    }

    case UserEntity::Premoving:
      setTags("validmove", to);
      return;

    default:
      break;
  }

  clearTags("validmove");
}

bool Board::dropOn(AbstractPiece::Ptr piece, const QPoint& point) {

  Point to = converter()->toLogical(point);
  if (!m_sprites.valid(to))
    return false;

  clearTags("validmove");

  switch(m_entity.lock()->validTurn(piece->color())) {

    case UserEntity::Moving: {
      DropUserMove m(piece, to);
      AbstractMove::Ptr mv = m_entity.lock()->testMove(m);
      if (mv)  {
          m_entity.lock()->executeMove(mv);
          return true;
      }
      break;
    }

    case UserEntity::Premoving: {
      DropUserMove m(piece, to);
      if (m_entity.lock()->testPremove(m)) {
        m_entity.lock()->addPremove(m);
        setPremove(m);
      }
      break;
    }

    default:
      break;
  }
  std::cout << "invalid move" << std::endl;
  emit error(InvalidMove);
  return false;
}

#include "board.moc"
