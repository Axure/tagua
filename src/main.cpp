/*
  Copyright (c) 2006 Paolo Capriotti <p.capriotti@sns.it>
            (c) 2006 Maurizio Monge <maurizio.monge@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <klocale.h>

#include "mainwindow.h"
#include "crash.h"
#include "common.h"

static const char description[] =
    I18N_NOOP("A generic board game interface");

static const char version[] = "0.9.1";

void trap() {
  printf("Press enter to quit.\n");

  char dummy[4096];
  fgets(dummy, 4096, stdin);
}

int main(int argc, char **argv) {
  KAboutData about("kboard", 0, ki18n("KBoard"), version, ki18n(description),
                    KAboutData::License_GPL,
                    ki18n("(C) 2006 Paolo Capriotti, Maurizio Monge"),
                    KLocalizedString(),
                    "http://kboard.sourceforge.net",
                    "p.capriotti@gmail.com");
  about.addAuthor(ki18n("Paolo Capriotti"), KLocalizedString(), "p.capriotti@gmail.com");
  about.addAuthor(ki18n("Maurizio Monge"), KLocalizedString(), "p.capriotti@gmail.com");
  about.addCredit(ki18n("Jani Huhtanen"), ki18n("Gaussian blur code"));
  about.addCredit(ki18n("Marcin Jakubowski"), ki18n("X11 taskbar flashing"));
  about.addCredit(ki18n("Rici Lake"), ki18n("funclib lua library"));

  KCmdLineArgs::init(argc, argv, &about);

  KCmdLineOptions options;
  KCmdLineArgs::addCmdLineOptions(options);
  KApplication app;

  installCrashHander();
  atexit(trap);

  QString data_dir = qgetenv("KBOARD_DATA");
  if (data_dir.isEmpty()) data_dir = "data";
  
  KGlobal::dirs()->addResourceDir("appdata", data_dir);
  KGlobal::dirs()->addResourceDir("icon", data_dir + "/pics");
  KIconLoader::global()->reconfigure(about.appName(), KGlobal::dirs());
  
  MainWindow* widget = new MainWindow;
  widget->show();

  return app.exec();
}

