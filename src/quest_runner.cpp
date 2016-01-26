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
#include "quest_runner.h"
#include "settings.h"
#include <QApplication>
#include <QMessageBox>
#include <QSize>

/**
 * @brief Creates a quest runner.
 * @param parent The parent object of the thread.
 */
QuestRunner::QuestRunner(QObject* parent) :
  QObject(parent),
  process(this) {

  // Connect to QProcess signals to know when the quest is running and finished.
  connect(&process, SIGNAL(started()),
          this, SIGNAL(running()));
  connect(&process, SIGNAL(finished(int)),
          this, SIGNAL(finished()));
  connect(&process, SIGNAL(error(QProcess::ProcessError)),
          this, SIGNAL(finished()));  // TODO show the error


  // TODO emit solarus_fatal
}

/**
 * @brief Destroys the quest runner.
 *
 * If a quest is running, terminates it.
 */
QuestRunner::~QuestRunner() {

  if (process.state() == QProcess::Running) {
    // Give a chance to the quest process to finish properly.
    process.terminate();
    if (!process.waitForFinished(1000)) {
      // Kill it after a delay.
      process.kill();
    }
  }
}

/**
 * @brief Creates and returns the list of arguments to pass to the process.
 * @param quest_path The path of the quest to run.
 */
QStringList QuestRunner::create_arguments(const QString& quest_path) {

  QStringList arguments;

  Settings settings;

  // -run quest_path
  arguments << "-run";
  arguments << quest_path;

  // no-audio
  if (settings.get_value_bool(Settings::no_audio)) {
    arguments << "-no-audio";
  }

  // video-acceleration
  const bool video_acceleration =
      settings.get_value_bool(Settings::video_acceleration);
  arguments << "-video-acceleration=" + QString(video_acceleration ? "yes" : "no");

  // win-console
  const bool win_console = settings.get_value_bool(Settings::win_console);
  arguments << "-win-console=" + QString(win_console ? "yes" : "no");

  // quest-size
  const QSize size = settings.get_value_size(Settings::quest_size);
  if (size.isValid()) {
    QString size_str = QString::number(size.width()) + "x" +
        QString::number(size.height());
    arguments << "-quest-size=" + size_str;
  }

  // Path of the quest.
  arguments << quest_path;

  return arguments;
}

/**
 * @brief Returns whether the quest is started.
 *
 * The quest is started as soon as you call start(),
 * but it then takes a slight delay for the process to actually load and run.
 * The signal running() is emitted when the process is running.
 *
 * @return @c true if the quest is started.
 */
bool QuestRunner::is_started() const {

  return process.state() != QProcess::NotRunning;
}

/**
 * @brief Returns whether the quest is running.
 * @return @c true if the quest is running.
 */
bool QuestRunner::is_running() const {

  return process.state() == QProcess::Running;
}

/**
 * @brief Runs a specific quest.
 * @param quest_path The path of the quest to run.
 * Does nothing if the path is empty or if a quest is already running.
 *
 * This function returns immediately.
 * The signal running() is emitted when the process actually runs.
 */
void QuestRunner::start(const QString& quest_path) {

  if (quest_path.isEmpty()) {
    return;
  }

  if (is_started()) {
    return;
  }

  // Run the solarus-quest-editor executable itself with the special option "-run quest_path".
  QStringList editor_arguments = QApplication::arguments();
  if (editor_arguments.isEmpty()) {
    QMessageBox::warning(
          nullptr,
          tr("Failed to run quest"),
          tr("Cannot start quest process: no program name")
    );
  }
  QString program_name = editor_arguments.at(0);
  QStringList arguments = create_arguments(quest_path);

  process.start(program_name, arguments);

}

/**
 * @brief Stops the current quest.
 *
 * Returns immediately.
 * The signal finished() is emitted when the process is finished.
 */
void QuestRunner::stop() {

  if (is_started()) {
    process.terminate();
  }
}
