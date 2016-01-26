/*
 * Copyright (C) 2014-2016 Christopho, Solarus - http://www.solarus-games.org
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
#include <solarus/lowlevel/Debug.h>
#include <solarus/Arguments.h>
#include <solarus/MainLoop.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QLibraryInfo>
#include <QStyleFactory>
#include <QTranslator>

namespace {

/**
 * @brief Runs the quest editor GUI.
 * @param argc Number of arguments of the command line.
 * @param argv Command-line arguments.
 * @return 0 in case of success.
 */
int run_editor_gui(int argc, char* argv[]) {

  QString cmd_quest_path;
  QString cmd_file_path;
  if (argc > 1) {
    // Quest to open initially.
    cmd_quest_path = argv[1];
    if (argc > 2) {
      // File to open in this quest.
      cmd_file_path = argv[2];
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

  // Determine the quest and files to open,
  // from the command line or from recent files.
  QString quest_path;
  QStringList file_paths;
  QString active_file_path;

  Settings settings;
  if (!cmd_quest_path.isEmpty()) {
    // Quest specified in the command line.
    quest_path = cmd_quest_path;
    if (!cmd_file_path.isEmpty()) {
      file_paths << cmd_file_path;
    }
  }
  else if (settings.get_value_bool("restore_last_files")) {
    // Restore the default quest if any.
    QStringList last_quests = settings.get_value_string_list(Settings::last_quests);
    if (!last_quests.isEmpty()) {
      quest_path = last_quests.first();
      file_paths = settings.get_value_string_list(Settings::last_files);
      active_file_path = settings.get_value_string(Settings::last_file);
    }
  }

  // Open the quest.
  if (!quest_path.isEmpty()) {
    window.open_quest(quest_path);

    if (window.get_quest().is_valid()) {

      // Open the tabs.
      for (const QString& file_path : file_paths) {
        window.open_file(window.get_quest(), file_path);
      }
      if (!active_file_path.isEmpty()) {
        // Restore the active tab.
        window.open_file(window.get_quest(), active_file_path);
      }
    }
  }

  window.show();

  return application.exec();
}

/**
 * @brief Runs a quest like the solarus_run executable does.
 * @param argc Number of arguments of the command line.
 * @param argv Command-line arguments.
 * @return 0 in case of success.
 */
int run_quest(int argc, char* argv[]) {

  // Show a popup in case of fatal error.
  Solarus::Debug::set_abort_on_die(true);

  // Run the Solarus main loop.
  const Solarus::Arguments args(argc, argv);
  Solarus::MainLoop(args).run();

  return 0;
}

}

/**
 * @brief Entry point of the quest editor.
 *
 * To run the editor GUI:
 *   solarus-quest-editor [quest_path [file_path]]
 * To directly run a quest (no GUI, similar to solarus_run):
 *   -run quest_path
 *
 * @param argc Number of arguments of the command line.
 * @param argv Command-line arguments.
 * @return 0 in case of success.
 */
int main(int argc, char* argv[]) {

  if (argc > 1 && QString(argv[1]) == "-run") {
    // Quest run mode.
    return run_quest(argc, argv);
  }
  else {
    // Editor GUI mode.
    return run_editor_gui(argc, argv);
  }
}
