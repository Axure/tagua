/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@gmail.com>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>
            
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "engine.h"

#include <QTextStream>

#define ENGINE_DEBUG
#ifdef ENGINE_DEBUG
  #include <KDebug>
#endif

using namespace boost;

Engine::Engine(const QString& path, const QStringList& arguments)
  : m_path(path)
, m_arguments(arguments)
,m_side(-1) {
  connect(&m_engine, SIGNAL(readyReadStandardOutput()), this, SLOT(processInput()));
  connect(&m_engine, SIGNAL(started()), this, SLOT(engineRunning()));
  connect(&m_engine, SIGNAL(finished(int, QProcess::ExitStatus)), 
          this, SIGNAL(lostConnection()));
}

Engine::~Engine() { }

void Engine::start() {
  if (!m_workPath.isNull())
    m_engine.setWorkingDirectory(m_workPath);

  m_engine.setOutputChannelMode(KProcess::OnlyStdoutChannel);
  m_engine.setProgram(m_path, m_arguments);
  m_engine.start();
  initializeEngine();
}

void Engine::engineRunning() {
  Q_ASSERT(m_engine.state() == QProcess::Running);
  while (!m_command_queue.empty()) {
    sendCommand(m_command_queue.front(), false);
    m_command_queue.pop();
  }
}

void Engine::sendCommand(const QString& command, bool echo) {
  if (echo && m_console)
    m_console->echo(command);
    
  if (m_engine.state() == QProcess::Running) {
    QTextStream os(&m_engine);
    os << command << "\n";
#ifdef ENGINE_DEBUG
    kDebug() << "" << m_side << ">" << command;
#endif 
  }
  else {
    m_command_queue.push(command);
  }
}

void Engine::processInput() {
  // process only full lines
  while (m_engine.canReadLine()) {
    QString line = m_engine.readLine();
    line.remove("\n").remove("\r");
#ifdef ENGINE_DEBUG
    kDebug() << m_side << "<" << line;
#endif
    if (m_console)
      m_console->displayText(line + "\n", 0);
    receivedCommand(line);
  }
}

void Engine::textNotify(const QString& text) {
  sendCommand(text, false);
}


void Engine::setNotifier(const shared_ptr<EngineNotifier>& notifier) {
  m_notifier = notifier;
}

void Engine::setConsole(const shared_ptr<Console>& console) {
  m_console = console;
}
