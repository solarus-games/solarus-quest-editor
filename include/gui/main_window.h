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
#ifndef SOLARUSEDITOR_MAIN_WINDOW_H
#define SOLARUSEDITOR_MAIN_WINDOW_H

#include "ui_main_window.h"
#include <QMainWindow>
#include <memory>

class Quest;
class QuestManager;

/**
 * @brief Main window of the quest editor.
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

public:

  MainWindow(QWidget* parent, QuestManager& quest_manager);

  QuestManager& get_quest_manager();

private slots:

  // Menu actions.
  void on_actionRun_quest_triggered();
  void on_actionExit_triggered();
  void on_actionLoad_quest_triggered();
  void on_actionSave_triggered();
  void on_actionClose_triggered();

  // Quest.
  void current_quest_changed(Quest& quest);

protected:

  virtual void closeEvent(QCloseEvent* event) override;

private:

  bool confirm_close();
  void update_title();

  std::unique_ptr<Ui::MainWindow> ui;
  QuestManager& quest_manager;

};

#endif
