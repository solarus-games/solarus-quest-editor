#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QMainWindow>
#include <memory>

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = nullptr);

  QString get_quest_path();
  bool set_quest_path(QString quest_path);

private slots:
  void on_actionRun_quest_triggered();

  void on_actionExit_triggered();

  void on_actionLoad_quest_triggered();

private:

  std::unique_ptr<Ui::MainWindow> ui;

  QString quest_path;      /**< Path of the quest loaded. */
};

#endif // MAINWINDOW_H
