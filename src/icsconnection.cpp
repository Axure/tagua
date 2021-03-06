/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@gmail.com>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "icsconnection.h"
#include <QRegExp>
#include <QStringList>
#include <KDebug>
#include "poolinfo.h"
#include "positioninfo.h"
#include "player.h"
#include "gameinfo.h"
#include "pgnparser.h"
#include "icslistener.h"
#include "variants.h"

using namespace boost;

QRegExp ICSConnection::pressReturn("^Press return to enter the server as \"\\S+\":");

// example: Creating: azsxdc (++++) Hispanico (1684) unrated crazyhouse 3 0
QRegExp ICSConnection::creating("^Creating: (\\S+)\\s+\\((\\S*)\\)\\s+(\\S+)\\s+\\((\\S*)\\)"
                              "\\s+(\\S+)\\s+(\\S+)\\s+(\\d+)\\s+(\\d+)\\s*.*$");

// example: {Game 149 (azsxdc vs. Hispanico) Creating unrated crazyhouse match.}
// example: {Game 149 (azsxdc vs. Hispanico) Game aborted on move 1} *
QRegExp ICSConnection::game("^\\{Game\\s+(\\d+)\\s+\\((\\S+)\\s+vs\\.\\s+(\\S+)\\)\\s+"
                                           "(\\S+.*)\\}(?:\\s+(\\S+.*)|\\s*)$");
QRegExp ICSConnection::unexamine("^You\\s+are\\s+no\\s+longer\\s+examining\\s+game\\s+(\\d+)");
QRegExp ICSConnection::unobserve("^Removing\\s+game\\s+(\\d+)\\s+from\\s+observation\\s+list\\s*");

// example: 124  848 shanmark    1155 damopn     [ br  2  12]   2:04 -  2:08 (39-39) W:  3
//QRegExp ICSConnection::gameInfo("^(\\d+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+"
//                                              "\\[\\s+(\\S+)\\s+(\\d+)\\s+(\\d+)\\s+\\]");

//                 1     2        3     4      5       6     7  8  9
// example: Game 124: shanmark ( 848) damopn (1155) rated blitz 2 12
//          Game 170: AstralVision (2167) vladx (2197) rated standard 15 0
QRegExp ICSConnection::observed_game("^Game\\s+(\\d+):\\s+(\\S+)\\s+\\(\\s*(\\S+)\\s*\\)\\s+"
                                                     "(\\S+)\\s+\\(\\s*(\\S+)\\s*\\)\\s+"
                                                     "(\\S+)\\s+(\\S+)\\s+(\\d+)\\s+(\\d+)");

QRegExp ICSConnection::login("^login: $");
QRegExp ICSConnection::password("^password: $");
QRegExp ICSConnection::fics("^\\S+% ");
QRegExp ICSConnection::beep("^\a");

//ex: Movelist for game 95:
QRegExp ICSConnection::move_list_start("^Movelist\\s+for\\s+game\\s+(\\d+):");
//ex: npssnt (1586) vs. mikkk (1487) --- Sun Sep  3, 18:16 PDT 2006
QRegExp ICSConnection::move_list_players("^(\\S+)\\s+\\((\\S*)\\)\\s+vs.\\s+(\\S+)\\s+\\((\\S*)\\)\\s*.*");
//ex: Unrated blitz match, initial time: 3 minutes, increment: 0 seconds.
QRegExp ICSConnection::move_list_game("^(\\S+)\\s+(\\S+)\\s+.*initial\\s+time:\\s*(\\d+)"
                                                       "\\s+.*increment:\\s*(\\d+)\\s*.*");
//ex: Move  npssnt             mikkk
QRegExp ICSConnection::move_list_ignore1("^Move\\s+(\\S+)\\s+(\\S+)");
//ex: ----  ----------------   ----------------
QRegExp ICSConnection::move_list_ignore2("^( +|-+)+");
QRegExp ICSConnection::move_list_terminator("^\\s*$");

QRegExp ICSConnection::goForward("^Game (\\d+): (\\S+) goes forward (\\d+) moves?\\.");
QRegExp ICSConnection::goBack("^Game (\\d+): (\\S+) backs up (\\d+) moves?\\.");


ICSConnection::ICSConnection()
: incomingGameInfo(0)
, m_move_list_game_info(NULL)
, m_move_list_position_info(NULL)
, m_move_list_pool_info(NULL) {
  state = Normal;
  connect(this, SIGNAL(receivedLine(QString, int)), this, SLOT(process(QString)));
  connect(this, SIGNAL(receivedText(QString, int)), this, SLOT(processPartialLine(QString)));
}

bool ICSConnection::test(const QRegExp& pattern, const QString& str) {
  if (pattern.indexIn(str, m_processed_offset, QRegExp::CaretAtOffset) >= 0) {
    m_processed_offset = pattern.matchedLength();
    return true;
  }
  else return false;
}

void ICSConnection::processPartialLine(QString str) {
//  kDebug() << "processing (partial) " << str;
  if (test(fics, str)) {
//    kDebug() << "matched prompt";
    prompt();
  }
  else if (test(beep, str)) {
    notification();
  }
  else if (test(pressReturn, str)) {
    registeredNickname();
  }
  else if (test(login, str)) {
    loginPrompt();
  }
  else if (test(password, str)) {
    passwordPrompt();
  }
  else {
    // no match, but it could be a partial command
    m_processed_offset = 0;
  }
}

void ICSConnection::process(QString str) {
  switch(state) {
  case Normal:
    if (test(creating, str)) {
      delete incomingGameInfo;
      incomingGameInfo = new GameInfo(creating, 1);
    }
    else if(test(observed_game, str)) {
      if(incomingGameInfo)
        delete incomingGameInfo;
      incomingGameInfo = new GameInfo(Player(observed_game.cap(2), observed_game.cap(3).toInt()),
                                          Player(observed_game.cap(4), observed_game.cap(5).toInt()),
                                          observed_game.cap(6), observed_game.cap(7),
                                          observed_game.cap(8).toInt(), observed_game.cap(9).toInt() );
      int number = observed_game.cap(1).toInt();
      incomingGameInfo->setGameNumber(number);
      m_games[number] = ICSGameData(-1, incomingGameInfo->type());
      //kDebug() << "ok, obs " << number << " of type " << incomingGameInfo->type();
    }
    else if (test(game, str)) {
      //kDebug() << "matched game. incomingGameInfo = " << incomingGameInfo;
      if(game.cap(4).startsWith("Creating") || game.cap(4).startsWith("Continuing") ) {
        if (!incomingGameInfo) {
          //this should really never happen, but anyway...
          QStringList info = game.cap(4).split(' ');
          if(info.size() >= 3)
            incomingGameInfo = new GameInfo(Player(game.cap(2), 0),
                                          Player(game.cap(3), 0),
                                          info[1], info[2], 0, 0 );
        }
        if (incomingGameInfo) {
          int number = game.cap(1).toInt();
          incomingGameInfo->setGameNumber(number);
          m_games.insert(std::make_pair(number, ICSGameData(-1, incomingGameInfo->type())));
        }
      }
      else {
        if (!incomingGameInfo) {
          int number = game.cap(1).toInt();
          //kDebug() << "matching game " << number << " end";
          m_games.erase(number);
          QString what = game.cap(4);
          QString result = game.cap(5);
          endingGame(what, result);
        }
      }
    }
    else if (test(unexamine, str)) {
      //kDebug() << "matching examined game end";
      int gameNumber = unexamine.cap(1).toInt();
      m_games.erase(gameNumber);
      endingExaminedGame(gameNumber);
    }
    else if (test(unobserve, str)) {
      int gameNumber = unobserve.cap(1).toInt();
      m_games.erase(gameNumber);
      endingObservedGame(gameNumber);
    }
    else if (test(move_list_start, str)) {
      //kDebug() << "entering move list state";
      m_move_list_game_num = move_list_start.cap(1).toInt();
      state = MoveListHeader;
    }
    else {
      PositionInfo positionInfo;
      bool game_start = positionInfo.load(m_games, str);
      if (positionInfo.valid) {
        int gameNumber = positionInfo.gameNumber;
        GameList::const_iterator game_it = m_games.find(gameNumber);
        Q_ASSERT(game_it != m_games.end());

        bool incoming = incomingGameInfo &&
                        incomingGameInfo->gameNumber() == gameNumber;
        
        if (game_start || incoming) {
          // delete extraneous game info
          if (incomingGameInfo &&
              incomingGameInfo->gameNumber() != gameNumber) {
            int n = incomingGameInfo->gameNumber();
            if (n != -1)
              m_games.erase(n);
            delete incomingGameInfo;
            incomingGameInfo = 0;
          }
          
          // no info on this game
          if (!incomingGameInfo) {
            kWarning() << "unexpected style 12 for game" << gameNumber;
            incomingGameInfo = new GameInfo(Player(positionInfo.whitePlayer, 0),
                                          Player(positionInfo.blackPlayer, 0),
                                          "rated", "", 0, 0);
          }
          
          switch (positionInfo.relation) {
            case PositionInfo::NotMyMove:
            case PositionInfo::MyMove:
              //kDebug() << "creating game";
              creatingGame(incomingGameInfo, positionInfo);
              break;
            case PositionInfo::Examining:
              //kDebug() << "creating examination";
              creatingExaminedGame(incomingGameInfo, positionInfo);
              break;
            case PositionInfo::ObservingPlayed:
            case PositionInfo::ObservingExamined:
              //kDebug() << "creating obs " << gameNumber << " " << incomingGameInfo->type();
              creatingObservedGame(incomingGameInfo, positionInfo);
              break;
            default:
              // unknown relation: ignoring
              break;
          }
          
          delete incomingGameInfo;
          incomingGameInfo = 0;
        }
        
        if (shared_ptr<ICSListener> listener = m_games[positionInfo.gameNumber].listener.lock())
          listener->notifyStyle12(positionInfo, game_start);
          
        if (positionInfo.relation == PositionInfo::MyMove) {
          notification();
        }
      }
      else {
        PoolInfo pool_info(m_games, str);
        if (pool_info.m_valid) {
          // BROKEN
          if (!pool_info.m_added_piece) {
            if (shared_ptr<ICSListener> listener = m_games[pool_info.m_game_num].listener.lock())
              listener->notifyPool(pool_info);
          }
        }
      }
    }
    break;

  case MoveListHeader:
    if (test(move_list_players, str)){
      //kDebug() << "move list players: " << str;
      m_move_list_players = move_list_players.capturedTexts();
    }
    else if (test(move_list_game, str)){
      //kDebug() << "move list game: " << str;
      if (m_move_list_game_info)
        delete m_move_list_game_info;

      if (m_move_list_players.size()>=5)
        m_move_list_game_info = new GameInfo(
                    Player(m_move_list_players[1], m_move_list_players[2].toInt()),
                    Player(m_move_list_players[3], m_move_list_players[4].toInt()),
                    move_list_game.cap(1).toLower(), move_list_game.cap(2),
                    move_list_game.cap(3).toInt(), move_list_game.cap(4).toInt()
                );
      else
        m_move_list_game_info = new GameInfo( Player("unknown",0), Player("unknown",0),
                    move_list_game.cap(1).toLower(), move_list_game.cap(2),
                    move_list_game.cap(3).toInt(), move_list_game.cap(4).toInt()
                );
      m_move_list_game_info->setGameNumber(m_move_list_game_num);

      //NOTE: here is where an unknown variant will be "upgraded" to the correct variant
      m_games[m_move_list_game_num].setType(move_list_game.cap(2));
    }
    else if (test(move_list_terminator, str)) {
      //kDebug() << "move list ign3: " << str;
    }
    else if (test(move_list_ignore1, str)){
      //kDebug() << "move list ign1: " << str;
    }
    else if (test(move_list_ignore2, str)) {
      //kDebug() << "move list ign2: " << str;
      state = MoveListMoves;
    }
    else {
      PositionInfo pi;
      pi.load(m_games, str);
      if (pi.valid)
        m_move_list_position_info = new PositionInfo(pi);
      else {
        PoolInfo pooli(m_games, str);
        if(pooli.m_valid)
          m_move_list_pool_info = new PoolInfo(pooli);
      }
    }
    break;
  case MoveListMoves:
    if (test(move_list_terminator, str)){
      if (shared_ptr<ICSListener> listener = m_games[m_move_list_game_num].listener.lock()) {
        AbstractPosition::Ptr p;
        if (m_move_list_position_info)
          p = m_move_list_position_info->position;
        else {
          std::map<int, ICSGameData>::const_iterator gi = m_games.find(m_move_list_game_num);
          if (gi == m_games.end()) {
            kError() << "Received move list for unknown game  " << m_move_list_game_num;
          }
          else {
            VariantPtr variant = gi->second.variant;
            p = variant->createPosition();
            p->setup();
          }
        }
        
        if (p) {
          if (m_move_list_pool_info) {
            //BROKEN
            //p->setPool(m_move_list_pool_info->m_pool);
          }
    
          PGN pgn(m_move_list);
          if (!pgn.valid())
            kDebug() << "parse error on move list";
          else
            listener->notifyMoveList(m_move_list_game_num, p, pgn);
        }
      }

      if (m_move_list_game_info)
        delete m_move_list_game_info;
      if (m_move_list_position_info)
        delete m_move_list_position_info;
      if (m_move_list_pool_info)
        delete m_move_list_pool_info;
      m_move_list_game_info = NULL;
      m_move_list_position_info = NULL;
      m_move_list_pool_info = NULL;
      m_move_list = QString();
      state = Normal;
    }
    else
      m_move_list += str;
    break;
  }

  m_processed_offset = 0;
}

void ICSConnection::setListener(int gameNumber, const weak_ptr<ICSListener>& listener) {
  m_games[gameNumber].listener = listener;
}

void ICSConnection::startup() {
  sendText("alias $ @");
  sendText("iset startpos 1");
  sendText("iset ms 1");
  sendText("iset lock 1");
  sendText("set interface Tagua-0.10 (http://www.tagua-project.org)");
  sendText("set style 12");
}

