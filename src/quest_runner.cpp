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
#include "quest_runner.h"
#include "settings.h"
#include <solarus/Arguments.h>
#include <solarus/MainLoop.h>
#include <solarus/SolarusFatal.h>
#include <solarus/lowlevel/Debug.h>
#include <solarus/lua/LuaContext.h>
#include <iostream>
#include <QSize>

/**
 * @brief Creates a quest runner.
 * @param parent The parent object of the thread.
 */
QuestRunner::QuestRunner(QObject *parent) :
  QThread(parent),
  quest_path(""),
  main_loop(nullptr) {

  Solarus::Debug::set_show_popup_on_die(false);
}

/**
 * @brief Returns the path of the current quest.
 * @return The path of the quest.
 */
QString QuestRunner::get_quest_path() const {
  return quest_path;
}

/**
 * @brief Returns the main loop of the current quest.
 * @return The main loop of the current quest or nullptr if no quest is running.
 */
MainLoop* QuestRunner::get_main_loop() const {
  return main_loop;
}

/**
 * @brief Runs a specific quest.
 * @param quest_path The path of the quest to run.
 * @param priority The priority of the thread.
 */
void QuestRunner::start(const QString& quest_path, Priority priority) {

  if (!isRunning()) {
    this->quest_path = quest_path;
    QThread::start(priority);
  }
}

/**
 * @brief Stops the current quest.
 */
void QuestRunner::stop() {

  if (main_loop != nullptr) {
    main_loop->set_exiting();
  }
}

/**
 * @brief Runs the quest.
 */
void QuestRunner::run() {

  if (quest_path.isEmpty()) {
    return;
  }

  Settings settings;

  try {

    Solarus::Arguments arguments;

    // no-audio.
    if (settings.get_value_bool(Settings::no_audio)) {
      arguments.add_argument("-no-audio");
    }

    // video-acceleration.
    bool video_acceleration =
      settings.get_value_bool(Settings::video_acceleration);
    arguments.add_argument(
      "-video-acceleration", video_acceleration ? "yes" : "no");

    // win-console.
    if (settings.get_value_bool(Settings::win_console)) {
      arguments.add_argument("-win-console", "yes");
    }

    // quest-size.
    QSize size = settings.get_value_size(Settings::quest_size);
    if (size.isValid()) {
      QString size_str = QString::number(size.width()) + "x" +
                         QString::number(size.height());
      arguments.add_argument("-quest-size", size_str.toStdString());
    }

    // Path of the quest.
    arguments.add_argument(quest_path.toStdString());

    main_loop = new MainLoop(arguments);
    main_loop->run();
  }
  catch (const Solarus::SolarusFatal& ex) {

    emit solarus_fatal(QString::fromStdString(ex.what()));
  }

  if (main_loop != nullptr) {
    delete main_loop;
    main_loop = nullptr;
  }
}
