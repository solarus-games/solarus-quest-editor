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
#include "gui/editor.h"
#include "gui/enum_menus.h"
#include "gui/external_script_dialog.h"
#include "gui/gui_tools.h"
#include "gui/main_window.h"
#include "entity_traits.h"
#include "file_tools.h"
#include "map_model.h"
#include "new_quest_builder.h"
#include "obsolete_editor_exception.h"
#include "obsolete_quest_exception.h"
#include "quest.h"
#include <solarus/Arguments.h>
#include <solarus/MainLoop.h>
#include <solarus/SolarusFatal.h>
#include <solarus/lowlevel/Debug.h>
#include <QActionGroup>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QUndoGroup>
#include <iostream>

using EntityType = Solarus::EntityType;

/**
 * @brief Creates a main window.
 * @param parent The parent widget or nullptr.
 */
MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  zoom_menu(nullptr),
  zoom_button(nullptr) {

  // Set up widgets.
  ui.setupUi(this);

  // Title.
  update_title();

  // Icon.
  QStringList icon_sizes = { "16", "32", "48", "256" };
  QIcon icon;
  for (const QString size : icon_sizes) {
    icon.addPixmap(":/images/icon_quest_editor_" + size + ".png");
  }
  setWindowIcon(icon);

  // Main splitter.
  const int tree_width = 300;
  ui.splitter->setSizes({ tree_width, width() - tree_width });

  // Menu and toolbar actions.
  QUndoGroup& undo_group = ui.tab_widget->get_undo_group();
  QAction* undo_action = undo_group.createUndoAction(this);
  undo_action->setIcon(QIcon(":/images/icon_undo.png"));
  QAction* redo_action = undo_group.createRedoAction(this);
  redo_action->setIcon(QIcon(":/images/icon_redo.png"));
  ui.menu_edit->insertAction(ui.action_cut, undo_action);
  ui.menu_edit->insertAction(ui.action_cut, redo_action);
  ui.menu_edit->insertSeparator(ui.action_cut);
  ui.tool_bar->insertAction(ui.action_run_quest, undo_action);
  ui.tool_bar->insertAction(ui.action_run_quest, redo_action);
  ui.tool_bar->insertSeparator(ui.action_run_quest);

  zoom_button = new QToolButton();
  zoom_button->setIcon(QIcon(":/images/icon_zoom.png"));
  zoom_button->setToolTip(tr("Zoom"));
  zoom_menu = create_zoom_menu();
  zoom_button->setMenu(zoom_menu);
  zoom_button->setPopupMode(QToolButton::InstantPopup);
  ui.tool_bar->insertWidget(ui.action_show_grid, zoom_button);
  ui.menu_view->insertMenu(ui.action_show_grid, zoom_menu);

  show_entities_button = new QToolButton();
  show_entities_button->setIcon(QIcon(":/images/icon_glasses.png"));
  show_entities_button->setToolTip(tr("Show entity types"));
  show_entities_menu = create_show_entities_menu();
  show_entities_button->setMenu(show_entities_menu);
  show_entities_button->setPopupMode(QToolButton::InstantPopup);
  ui.tool_bar->insertWidget(nullptr, show_entities_button);
  ui.menu_view->insertMenu(nullptr, show_entities_menu);

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

  connect(ui.tab_widget, SIGNAL(currentChanged(int)),
          this, SLOT(current_editor_changed(int)));
  connect(ui.tab_widget, SIGNAL(can_cut_changed(bool)),
          ui.action_cut, SLOT(setEnabled(bool)));
  connect(ui.tab_widget, SIGNAL(can_copy_changed(bool)),
          ui.action_copy, SLOT(setEnabled(bool)));
  connect(ui.tab_widget, SIGNAL(can_paste_changed(bool)),
          ui.action_paste, SLOT(setEnabled(bool)));

  // No editor initially.
  current_editor_changed(-1);
}

/**
 * @brief Returns the editor of the current tab, if any.
 * @return The current editor or nullptr.
 */
Editor* MainWindow::get_current_editor() {

  return ui.tab_widget->get_editor();
}

/**
 * @brief Creates a menu with zoom actions.
 * @return The created menu. It has no parent initially.
 */
QMenu* MainWindow::create_zoom_menu() {

  QMenu* zoom_menu = new QMenu(tr("Zoom"));
  std::vector<std::pair<QString, double>> zooms = {
    { tr("25 %"), 0.25 },
    { tr("50 %"), 0.5 },
    { tr("100 %"), 1.0 },
    { tr("200 %"), 2.0 },
    { tr("400 %"), 4.0 }
  };
  QActionGroup* action_group = new QActionGroup(this);
  for (const std::pair<QString, double>& zoom : zooms) {
    QAction* action = new QAction(zoom.first, action_group);
    zoom_actions[zoom.second] = action;
    action->setCheckable(true);
    connect(action, &QAction::triggered, [=]() {
      Editor* editor = ui.tab_widget->get_editor();
      if (editor != nullptr) {
        editor->set_zoom(zoom.second);
      }
    });
    zoom_menu->addAction(action);
  }

  return zoom_menu;
}

/**
 * @brief Creates a menu with actions to show each entity type.
 * @return The created menu. It has no parent initially.
 */
QMenu* MainWindow::create_show_entities_menu() {

  QMenu* menu = new QMenu(tr("Show entity types"));

  // Add show entity types actions to the menu.
  QList<QAction*> entity_actions = EnumMenus<EntityType>::create_actions(
        *menu,
        EnumMenuCheckableOption::CHECKABLE,
        [=](EntityType type) {
    Editor* editor = ui.tab_widget->get_editor();
    if (editor != nullptr) {
      QString type_name = EntityTraits::get_lua_name(type);
      const bool visible = show_entities_actions[type_name]->isChecked();
      editor->set_entity_type_visible(type, visible);
    }
  });

  for (QAction* action : entity_actions) {
    EntityType type = static_cast<EntityType>(action->data().toInt());
    if (!EntityTraits::can_be_stored_in_map_file(type)) {
      // Only show the ones that can exist in map files.
      menu->removeAction(action);
      continue;
    }
    QString type_name = EntityTraits::get_lua_name(type);
    show_entities_actions[type_name] = action;
  }

  // Add special actions Show all and Hide all.
  QAction* show_all_action = new QAction(tr("Show all"), this);
  show_entities_actions["action_show_all"] = show_all_action;
  menu->insertAction(entity_actions.first(), show_all_action);
  connect(show_all_action, &QAction::triggered, [=]() {
    Editor* editor = get_current_editor();
    if (editor != nullptr) {
      editor->show_all_entity_types();
    }
  });

  QAction* hide_all_action = new QAction(tr("Hide all"), this);
  show_entities_actions["action_hide_all"] = hide_all_action;
  menu->insertAction(entity_actions.first(), hide_all_action);
  connect(hide_all_action, &QAction::triggered, [=]() {
    Editor* editor = get_current_editor();
    if (editor != nullptr) {
      editor->hide_all_entity_types();
    }
  });

  menu->insertSeparator(entity_actions.first());

  return menu;
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
  ui.action_run_quest->setEnabled(false);
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

    ui.action_run_quest->setEnabled(true);
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
        upgrade_quest();
        // Reload the quest after upgrade.
        quest.set_root_path("");
        quest.set_root_path(quest_path);
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
 * @brief Attempts to upgrade the quest to the latest version and shows the
 * result in a dialog.
 */
void MainWindow::upgrade_quest() {

  // First backup the files.
  QString quest_version = quest.get_properties().get_solarus_version_without_patch();
  QString root_path = quest.get_root_path();
  QString backup_dir_name = "data." + quest_version + ".bak";
  QString backup_path = root_path + "/" + backup_dir_name;

  FileTools::delete_recursive(backup_path);  // Remove any previous backup.
  FileTools::copy_recursive(quest.get_data_path(), backup_path);

  // Upgrade data files.
  ExternalScriptDialog dialog(
        tr("Upgrading quest data files"),
        ":/quest_converter/update_quest",
        root_path);

  dialog.exec();
  bool upgrade_success = dialog.is_successful();

  if (!upgrade_success) {
    // The upgrade failed.
    // Restore the backuped version.
    QDir root_dir(root_path);
    FileTools::delete_recursive(root_path + "/data.err");
    root_dir.rename("data", "data.err");
    FileTools::delete_recursive(root_path + "/data");
    root_dir.rename(backup_dir_name, "data");

    throw EditorException(
          tr("An error occured while upgrading the quest.\n"
             "Your quest was kept unchanged in format %1.").arg(quest_version));
  }
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

  Editor* editor = get_current_editor();
  if (editor != nullptr) {
    editor->cut();
  }
}

/**
 * @brief Slot called when the user triggers the "Copy" action.
 */
void MainWindow::on_action_copy_triggered() {

  Editor* editor = get_current_editor();
  if (editor != nullptr) {
    editor->copy();
  }
}

/**
 * @brief Slot called when the user triggers the "Paste" action.
 */
void MainWindow::on_action_paste_triggered() {

  Editor* editor = get_current_editor();
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
 * @brief Slot called when the current editor changes.
 * @param index Index of the new current editor, or -1 if no editor is active.
 */
void MainWindow::current_editor_changed(int /* index */) {

  Editor* editor = get_current_editor();
  const bool has_editor = editor != nullptr;

  // Set up toolbar buttons for this editor.
  ui.action_cut->setEnabled(has_editor);
  ui.action_copy->setEnabled(has_editor);
  ui.action_paste->setEnabled(has_editor);
  ui.action_close->setEnabled(has_editor);
  ui.action_save->setEnabled(has_editor);

  const bool zoom_supported = has_editor && editor->is_zoom_supported();
  zoom_menu->setEnabled(zoom_supported);
  zoom_button->setEnabled(zoom_supported);

  const bool grid_supported = has_editor && editor->is_grid_supported();
  ui.action_show_grid->setEnabled(grid_supported);
  if (!grid_supported) {
    ui.action_show_grid->setChecked(false);
  }

  bool layer_visibility_supported =
      has_editor && editor->is_layer_visibility_supported();
  ui.action_show_layer_0->setEnabled(layer_visibility_supported);
  ui.action_show_layer_1->setEnabled(layer_visibility_supported);
  ui.action_show_layer_2->setEnabled(layer_visibility_supported);
  if (!layer_visibility_supported) {
    ui.action_show_layer_0->setChecked(false);
    ui.action_show_layer_1->setChecked(false);
    ui.action_show_layer_2->setChecked(false);
  }

  bool entity_type_visibility_supported =
      has_editor && editor->is_entity_type_visibility_supported();
  show_entities_menu->setEnabled(entity_type_visibility_supported);
  show_entities_button->setEnabled(entity_type_visibility_supported);

  if (has_editor) {

    connect(editor, SIGNAL(zoom_changed(double)),
            this, SLOT(editor_zoom_changed(double)));
    editor_zoom_changed(editor->get_zoom());

    connect(editor, SIGNAL(grid_visibility_changed(bool)),
            this, SLOT(editor_grid_visibility_changed(bool)));
    editor_grid_visibility_changed(editor->is_grid_visible());

    connect(editor, SIGNAL(layer_visibility_changed(Layer, bool)),
            this, SLOT(editor_layer_visibility_changed(Layer, bool)));
    editor_layer_visibility_changed();

    connect(editor, SIGNAL(entity_type_visibility_changed(EntityType, bool)),
            this, SLOT(editor_entity_type_visibility_changed(EntityType, bool)));
    editor_entity_type_visibility_changed();

  }
}

/**
 * @brief Slot called when the zoom of the current editor changes.
 * @param zoom The new zoom factor.
 */
void MainWindow::editor_zoom_changed(double zoom) {

  if (zoom_actions.contains(zoom)) {
    zoom_actions[zoom]->setChecked(true);
  }
}

/**
 * @brief Slot called when the grid of the current editor was just shown or hidden.
 * @param bool visible The new grid visiblity.
 */
void MainWindow::editor_grid_visibility_changed(bool grid_visible) {

  ui.action_show_grid->setChecked(grid_visible);
}

/**
 * @brief Slot called when a layer of the current editor was just shown or hidden.
 * @param layer The layer whose visibility has just changed.
 * @param bool visible The new layer visiblity.
 */
void MainWindow::editor_layer_visibility_changed(Layer layer, bool visible) {

  switch (layer) {

  case Layer::LAYER_LOW:
    ui.action_show_layer_0->setChecked(visible);
    break;

  case Layer::LAYER_INTERMEDIATE:
    ui.action_show_layer_1->setChecked(visible);
    break;

  case Layer::LAYER_HIGH:
    ui.action_show_layer_2->setChecked(visible);
    break;

  case Layer::LAYER_NB:
    qCritical() << "Invalid layer: " << layer;
    break;
  }
}

/**
 * @brief Slot called when the visibility of all layers should be updated to the GUI.
 */
void MainWindow::editor_layer_visibility_changed() {

  Editor* editor = get_current_editor();
  if (editor == nullptr) {
    return;
  }

  ui.action_show_layer_0->setChecked(editor->is_layer_visible(Layer::LAYER_LOW));
  ui.action_show_layer_1->setChecked(editor->is_layer_visible(Layer::LAYER_INTERMEDIATE));
  ui.action_show_layer_2->setChecked(editor->is_layer_visible(Layer::LAYER_HIGH));
}

/**
 * @brief Slot called when a entity type of the current editor was just shown or hidden.
 * @param entity_type The entity type whose visibility has just changed.
 * @param bool visible The new entity type visiblity.
 */
void MainWindow::editor_entity_type_visibility_changed(EntityType entity_type, bool visible) {

  Editor* editor = get_current_editor();
  if (editor == nullptr) {
    return;
  }

  QString type_name = EntityTraits::get_lua_name(entity_type);
  const auto& it = show_entities_actions.find(type_name);
  if (it == show_entities_actions.end() || it.value() == nullptr) {
    // This type of entity is not present in the menu.
    // This might be a type that cannot be used in the editor but only by the engine.
    return;
  }

  QAction* action = it.value();
  action->setChecked(visible);
}

/**
 * @brief Slot called when the visibility of all entity types should be updated to the GUI.
 */
void MainWindow::editor_entity_type_visibility_changed() {

  Editor* editor = get_current_editor();
  if (editor == nullptr) {
    return;
  }

  for (QAction* action: show_entities_actions) {
    if (action == nullptr) {
      qCritical() << "Missing show entity type action";
      return;
    }
    if (action->data().isValid()) {  // Skip special actions.
      EntityType entity_type = static_cast<EntityType>(action->data().toInt());
      action->setChecked(editor->is_entity_type_visible(entity_type));
    }
  }
}

/**
 * @brief Updates the title of the window from the current quest.
 */
void MainWindow::update_title() {

  QString version = SOLARUS_VERSION;
  QString title = tr("Solarus Quest Editor %1").arg(version);
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
