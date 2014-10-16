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
#include "gui/editor.h"
#include "gui/gui_tools.h"
#include "gui/main_window.h"
#include "quest.h"
#include "quest_manager.h"
#include <solarus/Arguments.h>
#include <solarus/MainLoop.h>
#include <solarus/SolarusFatal.h>
#include <solarus/lowlevel/Debug.h>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QUndoGroup>
#include <iostream>

/**
 * @brief Creates a main window.
 * @param parent The parent widget or nullptr.
 * @param quest_manager The quest manager.
 */
MainWindow::MainWindow(QWidget* parent, QuestManager& quest_manager) :
  QMainWindow(parent),
  ui(),
  quest_manager(quest_manager) {

  // Set up children widgets.
  ui.setupUi(this);

  const int tree_width = 300;
  ui.splitter->setSizes(QList<int>() << tree_width << width() - tree_width);

  QUndoGroup& undo_group = ui.tab_widget->get_undo_group();
  QAction* undo_action = undo_group.createUndoAction(this);
  QAction* redo_action = undo_group.createRedoAction(this);
  ui.menu_edit->insertAction(ui.action_cut, undo_action);
  ui.menu_edit->insertAction(ui.action_cut, redo_action);
  ui.menu_edit->insertSeparator(ui.action_cut);

  // Set standard keyboard shortcuts.
  ui.action_new_quest->setShortcut(QKeySequence::New);
  ui.action_load_quest->setShortcut(QKeySequence::Open);
  ui.action_close->setShortcut(QKeySequence::Close);
  ui.action_save->setShortcut(QKeySequence::Save);
  ui.action_exit->setShortcut(QKeySequence::Quit);
  undo_action->setShortcut(QKeySequence::Undo);
  redo_action->setShortcut(QKeySequence::Redo);
  ui.action_cut->setShortcut(QKeySequence::Cut);
  ui.action_copy->setShortcut(QKeySequence::Copy);
  ui.action_paste->setShortcut(QKeySequence::Paste);

  // Connect children.
  connect(ui.quest_tree_view, SIGNAL(open_file_requested(Quest&, const QString&)),
          ui.tab_widget, SLOT(open_file_requested(Quest&, const QString&)));

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
 * @brief Slot called when the user triggers the "Load quest" action.
 */
void MainWindow::on_action_load_quest_triggered() {

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
    GuiTools::error_dialog(
          tr("No quest was found in directory\n'%1'").arg(quest_path));
  }
}

/**
 * @brief Slot called when the user triggers the "Save" action.
 */
void MainWindow::on_action_save_triggered() {

  int index = ui.tab_widget->currentIndex();
  if (index == -1) {
    return;
  }
  ui.tab_widget->save_file_requested(index);
}

/**
 * @brief Slot called when the user triggers the "Close" action.
 */
void MainWindow::on_action_close_triggered() {

  int index = ui.tab_widget->currentIndex();
  if (index == -1) {
    return;
  }
  ui.tab_widget->close_file_requested(index);
}

/**
 * @brief Slot called when the user triggers the "Exit" action.
 */
void MainWindow::on_action_exit_triggered() {

  if (confirm_close()) {
    QApplication::exit(0);
  }
}

/**
 * @brief Slot called when the user triggers the "Cut" action.
 */
void MainWindow::on_action_cut_triggered() {

  Editor* editor = ui.tab_widget->get_editor();
  if (editor != nullptr) {
    editor->cut();
  }
}

/**
 * @brief Slot called when the user triggers the "Copy" action.
 */
void MainWindow::on_action_copy_triggered() {

  Editor* editor = ui.tab_widget->get_editor();
  if (editor != nullptr) {
    editor->copy();
  }
}

/**
 * @brief Slot called when the user triggers the "Paste" action.
 */
void MainWindow::on_action_paste_triggered() {

  Editor* editor = ui.tab_widget->get_editor();
  if (editor != nullptr) {
    editor->paste();
  }
}

/**
 * @brief Slot called when the user triggers the "Run quest" action.
 */
void MainWindow::on_action_run_quest_triggered() {

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
    std::cout << tr("Quest terminated unexpectedly: %1").arg(ex.what()).toStdString()
              << std::endl;
  }
}

/**
 * @brief Slot called when the user opens another quest.
 */
void MainWindow::current_quest_changed(Quest& quest) {

  update_title();

  ui.quest_tree_view->set_quest(quest);

  connect(&quest, SIGNAL(file_renamed(const QString&, const QString&)),
          ui.tab_widget, SLOT(file_renamed(const QString&, const QString&)));
  connect(&quest, SIGNAL(file_deleted(const QString&)),
          ui.tab_widget, SLOT(file_deleted(const QString&)));
}

/**
 * @brief Updates the title of the window from the current quest.
 */
void MainWindow::update_title() {

  QString title = tr("Solarus Quest Editor");
  QString quest_name = quest_manager.get_quest().get_name();
  if (!quest_name.isEmpty()) {
    title = quest_name + " - " + title;
  }

  setWindowTitle(title);
}

/**
 * @brief Opens a file of a quest if it exists.
 * @param quest A quest.
 * @param path The file to open.
 */
void MainWindow::open_file(Quest& quest, const QString& path) {

  ui.tab_widget->open_file_requested(quest, path);
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

  return ui.tab_widget->confirm_close();
}
