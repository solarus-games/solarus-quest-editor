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
#include "gui/gui_tools.h"
#include "gui/main_window.h"
#include "quest_manager.h"
#include <solarus/Arguments.h>
#include <solarus/MainLoop.h>
#include <solarus/SolarusFatal.h>
#include <solarus/lowlevel/Debug.h>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>

/**
 * @brief Creates a main window.
 * @param parent The parent widget or nullptr.
 * @param quest_manager The quest manager.
 */
MainWindow::MainWindow(QWidget* parent, QuestManager& quest_manager) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  quest_manager(quest_manager) {

  ui->setupUi(this);

  // Set up children.
  ui->quest_tree_view->set_quest_manager(quest_manager);

  int tree_width = 300;
  ui->splitter->setSizes(QList<int>() << tree_width << width() - tree_width);

  // Connect children.
  connect(ui->quest_tree_view, SIGNAL(open_file_requested(Quest&, const QString&)),
          ui->tabWidget, SLOT(open_file_requested(Quest&, const QString&)));

  // Connect to external signals.
  connect(&quest_manager, SIGNAL(current_quest_changed(Quest&)),
          this, SLOT(current_quest_changed(Quest&)));
}

/**
 * @brief Returns the quest manager.
 * @return The quest manager.
 */
QuestManager& MainWindow::get_quest_manager() {
  return quest_manager;
}

/**
 * @brief Slot called when the user triggers the "Exit" action.
 */
void MainWindow::on_actionExit_triggered() {

  if (confirm_close()) {
    QApplication::exit(0);
  }
}

/**
 * @brief Slot called when the user triggers the "Load quest" action.
 */
void MainWindow::on_actionLoad_quest_triggered() {

  QFileDialog dialog(nullptr, tr("Select quest directory"));
  dialog.setFileMode(QFileDialog::Directory);
  dialog.setOption(QFileDialog::ShowDirsOnly);
  if (!dialog.exec()) {
    return;
  }

  QStringList file_names = dialog.selectedFiles();
  if (file_names.empty()) {
    return;
  }

  QString quest_path = file_names.first();
  if (!quest_manager.set_quest(quest_path)) {
    GuiTools::errorDialog(
          "No quest was found in directory\n'" + quest_path + "'");
  }
}

/**
 * @brief Slot called when the user triggers the "Save" action.
 */
void MainWindow::on_actionSave_triggered() {

  ui->tabWidget->save_current_file();
}

/**
 * @brief Slot called when the user triggers the "Close" action.
 */
void MainWindow::on_actionClose_triggered() {

  ui->tabWidget->close_file_requested(ui->tabWidget->currentIndex());
}

/**
 * @brief Slot called when the user triggers the "Undo" action.
 */
void MainWindow::on_actionUndo_triggered() {

  ui->tabWidget->undo();
}

/**
 * @brief Slot called when the user triggers the "Redo" action.
 */
void MainWindow::on_actionRedo_triggered() {

  ui->tabWidget->redo();
}

/**
 * @brief Slot called when the user triggers the "Run quest" action.
 */
void MainWindow::on_actionRun_quest_triggered() {

  QString quest_path = quest_manager.get_quest().get_root_path();
  if (quest_path.isEmpty()) {
    return;
  }

  // TODO run quest in a separate thread
  try {
    Solarus::Arguments arguments;
    arguments.add_argument(quest_path.toStdString());
    Solarus::MainLoop main_loop(arguments);
    main_loop.run();
  }
  catch (const Solarus::SolarusFatal& ex) {
    // The run did not end well.
    std::cout << "Quest terminated unexpectedly: " << ex.what() << std::endl;
  }
}

/**
 * @brief Slot called when the user opens another quest.
 */
void MainWindow::current_quest_changed(Quest& /* quest */) {
  update_title();
}

/**
 * @brief Updates the title of the window from the current quest.
 */
void MainWindow::update_title() {

  QString title = "Solarus Quest Editor";
  QString quest_name = quest_manager.get_quest().get_name();
  if (!quest_name.isEmpty()) {
    title = quest_name + " - " + title;
  }

  setWindowTitle(title);
}

/**
 * @brief Receives a window close event.
 * @param event The event to handle.
 */
void MainWindow::closeEvent(QCloseEvent* event) {

  if (confirm_close()) {
    event->accept();
  }
  else {
    event->ignore();
  }
}

/**
 * @brief Function called when the user wants to exit the program.
 *
 * The user can save files if necessary.
 *
 * @return @c false to cancel the closing operation.
 */
bool MainWindow::confirm_close() {

  return ui->tabWidget->confirm_close();
}
