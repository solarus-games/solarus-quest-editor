#ifndef SOLARUSEDITOR_MAINWINDOW_H
#define SOLARUSEDITOR_MAINWINDOW_H

#include "ui_main_window.h"
#include <QMainWindow>
#include <memory>

class QuestManager;

/**
 * @brief Main window of the quest editor.
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

public:

  explicit MainWindow(QWidget* parent, QuestManager& quest_manager);

  QuestManager& get_quest_manager();

private slots:

  // Menu actions.
  void on_actionRun_quest_triggered();
  void on_actionExit_triggered();
  void on_actionLoad_quest_triggered();

  // Quest.
  void current_quest_changed(QString quest_path);

private:

  void update_title();

  std::unique_ptr<Ui::MainWindow> ui;
  QuestManager& quest_manager;

};

#endif
