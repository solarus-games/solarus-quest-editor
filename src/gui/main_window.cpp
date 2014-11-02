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
#include "obsolete_editor_exception.h"
#include "obsolete_quest_exception.h"
#include "new_quest_builder.h"
#include "quest.h"
#include <solarus/Arguments.h>
#include <solarus/MainLoop.h>
#include <solarus/SolarusFatal.h>
#include <solarus/lowlevel/Debug.h>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QUndoGroup>
#include <iostream>

/**
 * @brief Creates a main window.
 * @param parent The parent widget or nullptr.
 */
MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent) {

  // Set up children widgets.
  ui.setupUi(this);

  const int tree_width = 300;
  ui.splitter->setSizes({ tree_width, width() - tree_width });

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
}

/**
 * @brief Set an appropriate size and centers the window on the screen having
 * the mouse.
 */
void MainWindow::initialize_geometry_on_screen() {

  QDesktopWidget* desktop = QApplication::desktop();
  QRect screen = desktop->screenGeometry(desktop->screenNumber(QCursor::pos()));

  // Choose a comfortable initial size depending on the screen resolution.
  // The ui is designed to work well with a window size of 1280x680 and above.
  int width = 1270;
  int height = 680;
  if (screen.width() >= 1920) {
    width = 1500;
  }
  if (screen.height() >= 1024) {
    height = 980;
  }
  setGeometry(0, 0, qMin(width, screen.width()), qMin(height, screen.height()));

  // And center the window on the screen where the mouse is currently.
  int x = screen.width() / 2 - frameGeometry().width() / 2 + screen.left() - 2;
  int y = screen.height() / 2 - frameGeometry().height() / 2 + screen.top() - 10;

  move(qMax(0, x), qMax(0, y));
}

/**
 * @brief Returns the current quest open in the window.
 * @return The current quest, or an invalid quest if no quest is open.
 */
Quest& MainWindow::get_quest() {
  return quest;
}

/**
 * @brief Closes the current quest if any.
 */
void MainWindow::close_quest() {

  if (quest.exists()) {
    disconnect(&quest, SIGNAL(file_renamed(QString, QString)),
               ui.tab_widget, SLOT(file_renamed(QString, QString)));
    disconnect(&quest, SIGNAL(file_deleted(QString)),
               ui.tab_widget, SLOT(file_deleted(QString)));
  }

  quest.set_root_path("");
  update_title();
  ui.quest_tree_view->set_quest(quest);
}

/**
 * @brief Opens a quest.
 *
 * Shows an error dialog if the quest could not be opened.
 *
 * @param quest_path Path of the quest to open.
 * @return @c true if the quest was successfully opened.
 */
bool MainWindow::open_quest(const QString& quest_path) {

  // Close the previous quest.
  close_quest();

  // Load the requested quest.
  quest.set_root_path(quest_path);

  bool success = false;

  try {
    if (!quest.exists()) {
      throw EditorException(tr("No quest was found in directory\n'%1'").arg(quest_path));
    }
    quest.check_version();

    connect(&quest, SIGNAL(file_renamed(QString, QString)),
            ui.tab_widget, SLOT(file_renamed(QString, QString)));
    connect(&quest, SIGNAL(file_deleted(QString)),
            ui.tab_widget, SLOT(file_deleted(QString)));

    success = true;
  }
  catch (const ObsoleteEditorException& ex) {
    ex.show_dialog();
  }
  catch (const ObsoleteQuestException& ex) {
    // Quest data files are obsolete: upgrade them and try again.
    QMessageBox::StandardButton answer = QMessageBox::information(
          this,
          tr("Obsolete quest"),
          tr("The format of this quest (%1) is outdated.\n"
             "Your data files will be automatically updated to Solarus %2.").
          arg(ex.get_quest_format(), SOLARUS_VERSION_WITHOUT_PATCH),
          QMessageBox::Ok | QMessageBox::Cancel);

    if (answer == QMessageBox::Ok) {
      try {
        quest.upgrade();
        quest.check_version();
        success = true;
      }
      catch (const EditorException& ex) {
        // Upgrade failed.
        ex.show_dialog();
      }
    }
  }
  catch (const EditorException& ex) {
    ex.show_dialog();
  }

  if (!success) {
    quest.set_root_path("");
  }

  update_title();
  ui.quest_tree_view->set_quest(quest);

  return false;
}

/**
 * @brief Slot called when the user triggers the "New quest" action.
 */
void MainWindow::on_action_new_quest_triggered() {

  QString quest_path = QFileDialog::getExistingDirectory(
        this,
        tr("Select quest directory"),
        "",  // Initial value: current directory.
        QFileDialog::ShowDirsOnly);

  if (quest_path.isEmpty()) {
    return;
  }

  try {
    NewQuestBuilder::create_initial_quest_files(quest_path);
    if (open_quest(quest_path)) {
      QMessageBox::information(this, tr("Quest created"), tr(
                                 "Quest successfully created!\n"
                                 "The next step is to manually edit your quest properties in quest.dat\n"
                                 "(sorry, this is not fully supported by the editor yet).\n"));
    }
  }
  catch (const EditorException& ex) {
    ex.show_dialog();
  }

}

/**
 * @brief Slot called when the user triggers the "Load quest" action.
 */
void MainWindow::on_action_load_quest_triggered() {

  QString quest_path = QFileDialog::getExistingDirectory(
        this,
        tr("Select quest directory"),
        "",  // Initial value: current directory.
        QFileDialog::ShowDirsOnly);

  if (quest_path.isEmpty()) {
    return;
  }

  open_quest(quest_path);
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

  QString quest_path = quest.get_root_path();
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
 * @brief Updates the title of the window from the current quest.
 */
void MainWindow::update_title() {

  QString title = tr("Solarus Quest Editor");
  QString quest_name = quest.get_name();
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
