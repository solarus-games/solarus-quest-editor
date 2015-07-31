/*
 * Copyright (C) 2014-2015 Christopho, Solarus - http://www.solarus-games.org
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
#include "gui/main_window.h"
#include "settings.h"
#include "version.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QLibraryInfo>
#include <QStyleFactory>
#include <QTranslator>

/**
 * @brief Entry point of the quest editor.
 *
 * Arguments: [quest_path [file_path]]
 *
 * @param argc Number of arguments of the command line.
 * @param argv Command-line arguments.
 * @return 0 in case of success.
 */
int main(int argc, char* argv[]) {

  QString quest_path;
  QString file_path;
  if (argc > 1) {
    // Quest to open initially.
    quest_path = argv[1];
    if (argc > 2) {
      // File to open in this quest.
      file_path = argv[2];
    }
  }

  QApplication application(argc, argv);
  Settings::load_default_application_settings();

  // Set up the translations.
  QTranslator qt_translator;
  qt_translator.load("qt_" + QLocale::system().name(),
                     QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  application.installTranslator(&qt_translator);

  QTranslator translator;
  translator.load("solarus_editor_" + QLocale::system().name());
  application.installTranslator(&translator);

  MainWindow window(nullptr);

  // Choose an appropriate initial window size and position.
  window.initialize_geometry_on_screen();

  // Open the quest.
  if (!quest_path.isEmpty()) {
    window.open_quest(quest_path);

    if (window.get_quest().is_valid() && !file_path.isEmpty()) {
      window.open_file(window.get_quest(), file_path);
    }
  }

  window.show();

  return application.exec();
}
