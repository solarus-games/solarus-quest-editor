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
#ifndef SOLARUSEDITOR_QUEST_RUNNER_H
#define SOLARUSEDITOR_QUEST_RUNNER_H

#include <solarus/MainLoop.h>
#include <QThread>

using MainLoop = Solarus::MainLoop;

/**
 * @brief Class to run a quest in a dedicated thread.
 */
class QuestRunner : public QThread {
  Q_OBJECT

public:

  QuestRunner(QObject* parent = nullptr);

  QString get_quest_path() const;
  MainLoop* get_main_loop() const;

public slots:

  void start(const QString& quest_path, Priority priority = InheritPriority);
  void stop();

signals:

  void solarus_fatal(const QString& what);

private:

  void run() override;

  QString quest_path;   /**< The path of the quest to run. */
  MainLoop* main_loop;  /**< The main loop of the current quest. */

};

#endif
