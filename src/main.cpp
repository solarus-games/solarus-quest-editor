/*
 * Copyright (C) 2014 Christopho, Solarus - http://www.solarus-games.org
 *
 * Solarus Quest Editor is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Solarus Quest Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "quest_manager.h"
#include "gui/main_window.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QLibraryInfo>
#include <QTranslator>

/**
 * @brief Entry point of the quest editor.
 *
 * The argument if any is the path of the quest to load.
 *
 * @param argc Number of arguments of the command line.
 * @param argv Command-line arguments.
 * @return 0 in case of success.
 */
int main(int argc, char* argv[]) {

  QString quest_path;
  if (argc > 1) {
    quest_path = argv[1];
  }

  QApplication application(argc, argv);

  // Set up the translation.
  QTranslator translator;
  translator.load("solarus_editor_" + QLocale::system().name());
  application.installTranslator(&translator);

  QuestManager quest_manager;
  MainWindow window(nullptr, quest_manager);

  // Center the window initially.
  window.setGeometry(
        QStyle::alignedRect(
          Qt::LeftToRight,
          Qt::AlignCenter,
          window.size(),
          application.desktop()->availableGeometry()
          )
        );

  // Open the quest.
  quest_manager.set_quest(quest_path);

  window.show();

  return application.exec();
}
