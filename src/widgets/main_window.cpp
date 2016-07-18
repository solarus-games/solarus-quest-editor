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
#include "entities/entity_traits.h"
#include "widgets/editor.h"
#include "widgets/enum_menus.h"
#include "widgets/external_script_dialog.h"
#include "widgets/gui_tools.h"
#include "widgets/main_window.h"
#include "widgets/pair_spin_box.h"
#include "file_tools.h"
#include "map_model.h"
#include "new_quest_builder.h"
#include "obsolete_editor_exception.h"
#include "obsolete_quest_exception.h"
#include "quest.h"
#include "sound.h"
#include "version.h"
#include <solarus/gui/quest_runner.h>
#include <QActionGroup>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QUndoGroup>

namespace SolarusEditor {

using EntityType = Solarus::EntityType;

/**
 * @brief Creates a main window.
 * @param parent The parent widget or nullptr.
 */
MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  quest_runner(),
  recent_quests_menu(nullptr),
  zoom_menu(nullptr),
  zoom_button(nullptr),
  zoom_actions(),
  show_layers_menu(nullptr),
  show_layers_button(nullptr),
  show_layers_action(nullptr),
  show_layers_subactions(),
  show_entities_menu(nullptr),
  show_entities_button(nullptr),
  show_entities_subactions(),
  common_actions(),
  settings_dialog(this) {

  // Set up widgets.
  ui.setupUi(this);

  // Title.
  update_title();

  // Icon.
  QStringList icon_sizes = { "16", "32", "48", "256" };
  QIcon icon;
  Q_FOREACH (const QString size, icon_sizes) {
    icon.addPixmap(":/images/icon_quest_editor_" + size + ".png");
  }
  setWindowIcon(icon);

  // Quest tree splitter.
  const int tree_width = 300;
  ui.quest_tree_splitter->setSizes({ tree_width, width() - tree_width });

  // Console splitter.
  const int console_height = 100;
  ui.console_splitter->setSizes({ height() - console_height, console_height });
  ui.console_widget->setVisible(false);
  ui.console_widget->set_quest_runner(quest_runner);

  // Menu and toolbar actions.
  recent_quests_menu = new QMenu(tr("Recent quests"));
  update_recent_quests_menu();
  ui.menu_quest->insertMenu(ui.menu_quest->actions()[3], recent_quests_menu);

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
  ui.action_run_quest->setEnabled(false);

  ui.action_pause_music->setEnabled(false);
  ui.action_stop_music->setEnabled(false);

  zoom_button = new QToolButton();
  zoom_button->setIcon(QIcon(":/images/icon_zoom.png"));
  zoom_button->setToolTip(tr("Zoom"));
  zoom_menu = create_zoom_menu();
  zoom_button->setMenu(zoom_menu);
  zoom_button->setPopupMode(QToolButton::InstantPopup);
  ui.tool_bar->insertWidget(ui.action_show_grid, zoom_button);
  ui.menu_view->insertMenu(ui.action_show_grid, zoom_menu);

  grid_size = new PairSpinBox();
  grid_size->config("x", 8, 99999, 8);
  grid_size->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

  ui.tool_bar->insertWidget(ui.action_show_layer_0, grid_size);
  ui.tool_bar->insertSeparator(ui.action_show_layer_0);

  ui.action_show_layer_0->setShortcutContext(Qt::WidgetShortcut);
  ui.action_show_layer_1->setShortcutContext(Qt::WidgetShortcut);
  ui.action_show_layer_2->setShortcutContext(Qt::WidgetShortcut);
  show_layers_button = new QToolButton();
  show_layers_button->setIcon(QIcon(":/images/icon_layer_more.png"));
  show_layers_button->setToolTip(show_layers_button->text());
  show_layers_menu = new QMenu(tr("Show/hide more layers"));
  show_layers_button->setMenu(show_layers_menu);
  show_layers_button->setPopupMode(QToolButton::InstantPopup);
  show_layers_action = ui.tool_bar->addWidget(show_layers_button);
  ui.menu_view->addMenu(show_layers_menu);
  ui.menu_view->addSeparator();

  show_entities_button = new QToolButton();
  show_entities_button->setIcon(QIcon(":/images/icon_glasses.png"));
  show_entities_button->setToolTip(tr("Show/hide entity types"));
  show_entities_menu = create_show_entities_menu();
  show_entities_button->setMenu(show_entities_menu);
  show_entities_button->setPopupMode(QToolButton::InstantPopup);
  ui.tool_bar->addWidget(show_entities_button);
  ui.menu_view->addMenu(show_entities_menu);

  common_actions["cut"] = ui.action_cut;
  common_actions["copy"] = ui.action_copy;
  common_actions["paste"] = ui.action_paste;
  common_actions["undo"] = undo_action;
  common_actions["redo"] = redo_action;

  // Set standard keyboard shortcuts.
  ui.action_new_quest->setShortcut(QKeySequence::New);
  ui.action_close->setShortcut(QKeySequence::Close);
  ui.action_save->setShortcut(QKeySequence::Save);
  ui.action_exit->setShortcut(QKeySequence::Quit);
  undo_action->setShortcut(QKeySequence::Undo);
  redo_action->setShortcut(QKeySequence::Redo);
  ui.action_cut->setShortcut(QKeySequence::Cut);
  ui.action_copy->setShortcut(QKeySequence::Copy);
  ui.action_paste->setShortcut(QKeySequence::Paste);
  ui.action_select_all->setShortcut(QKeySequence::SelectAll);
  ui.action_unselect_all->setShortcut(QKeySequence::Deselect);
  ui.action_find->setShortcut(QKeySequence::Find);

  // Connect children.
  connect(ui.quest_tree_view, SIGNAL(open_file_requested(Quest&, QString)),
          ui.tab_widget, SLOT(open_file_requested(Quest&, QString)));
  connect(ui.quest_tree_view, SIGNAL(rename_file_requested(Quest&, QString)),
          this, SLOT(rename_file_requested(Quest&, QString)));

  connect(ui.tab_widget, SIGNAL(currentChanged(int)),
          this, SLOT(current_editor_changed(int)));
  connect(ui.tab_widget, SIGNAL(can_cut_changed(bool)),
          ui.action_cut, SLOT(setEnabled(bool)));
  connect(ui.tab_widget, SIGNAL(can_copy_changed(bool)),
          ui.action_copy, SLOT(setEnabled(bool)));
  connect(ui.tab_widget, SIGNAL(can_paste_changed(bool)),
          ui.action_paste, SLOT(setEnabled(bool)));

  connect(grid_size, SIGNAL(value_changed(int,int)),
          this, SLOT(change_grid_size()));

  connect(&quest_runner, SIGNAL(running()),
          this, SLOT(quest_running()));
  connect(&quest_runner, SIGNAL(finished()),
          this, SLOT(quest_finished()));

  connect(&quest, SIGNAL(current_music_changed(QString)),
          this, SLOT(current_music_changed(QString)));

  connect(&settings_dialog, SIGNAL(settings_changed()),
          this, SLOT(reload_settings()));

  // No editor initially.
  current_editor_changed(-1);
}

/**
 * @brief Destructor of main window.
 */
MainWindow::~MainWindow() {

}

/**
 * @brief Returns the editor of the current tab, if any.
 * @return The current editor or nullptr.
 */
Editor* MainWindow::get_current_editor() {

  return ui.tab_widget->get_editor();
}

/**
 * @brief Builds or rebuilds the recent quests menu.
 *
 * Disables it if there is no recent quest.
 */
void MainWindow::update_recent_quests_menu() {

  if (recent_quests_menu == nullptr) {
    return;
  }

  // Get the recent quest list.
  EditorSettings settings;
  QStringList last_quests = settings.get_value_string_list(EditorSettings::last_quests);

  // Clear previous actions.
  recent_quests_menu->clear();

  // Disable if there is no recent quest.
  recent_quests_menu->setEnabled(!last_quests.isEmpty());

  // Create new actions.
  Q_FOREACH (const QString& quest_path, last_quests) {

    QAction* action = new QAction(quest_path, recent_quests_menu);
    connect(action, &QAction::triggered, [this, quest_path]() {

      // Close the previous quest and open the new one.
      if (confirm_before_closing()) {
        close_quest();
        open_quest(quest_path);
      }
    });

    recent_quests_menu->addAction(action);
  }
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
    connect(action, &QAction::triggered, [this, zoom]() {
      Editor* editor = get_current_editor();
      if (editor != nullptr) {
        editor->get_view_settings().set_zoom(zoom.second);
      }
    });
    zoom_menu->addAction(action);
  }

  return zoom_menu;
}

/**
 * @brief Updates the show layers menu to match the current range of layers.
 */
void MainWindow::update_show_layers_menu() {

  if (show_layers_menu == nullptr) {
    return;
  }

  // Clear previous actions.
  const QList<QAction*> actions = show_layers_menu->actions();
  Q_FOREACH (QAction* action, actions) {
    delete action;
  }
  show_layers_menu->clear();

  // Add special actions Show all and Hide all.
  QAction* show_all_action = new QAction(tr("Show all layers"), this);
  show_layers_subactions["action_show_all"] = show_all_action;
  show_layers_menu->addAction(show_all_action);
  connect(show_all_action, &QAction::triggered, [this]() {
    Editor* editor = get_current_editor();
    if (editor != nullptr) {
      editor->get_view_settings().show_all_layers();
    }
  });

  QAction* hide_all_action = new QAction(tr("Hide all layers"), this);
  show_layers_subactions["action_hide_all"] = hide_all_action;
  show_layers_menu->addAction(hide_all_action);
  connect(hide_all_action, &QAction::triggered, [this]() {
    Editor* editor = get_current_editor();
    if (editor != nullptr) {
      editor->get_view_settings().hide_all_layers();
    }
  });

  show_layers_menu->addSeparator();

  // Add an action for each layer.
  Editor* editor = get_current_editor();
  if (editor != nullptr) {
    int min_layer = 0;
    int max_layer = 0;
    editor->get_layers_supported(min_layer, max_layer);
    for (int i = min_layer; i <= max_layer; ++i) {
      QAction* action = new QAction(tr("Show layer %1").arg(i), this);
      if (i >= 0 && i < 3) {
        // Layers 0, 1 and 2 have an icon.
        QString file_name = QString(":/images/icon_layer_%1.png").arg(i);
        action->setIcon(QIcon(file_name));
      }
      if (i >= 0 && i <= 9) {
        // Layers 0 to 9 have a shortcut.
        action->setShortcut(QString::number(i));
      }
      action->setCheckable(true);
      action->setChecked(true);
      show_layers_menu->addAction(action);
      connect(action, &QAction::triggered, [this, action, i]() {
        Editor* editor = get_current_editor();
        if (editor != nullptr) {
          const bool visible = action->isChecked();
          editor->get_view_settings().set_layer_visible(i, visible);
        }
      });
    }
  }
}

/**
 * @brief Creates a menu with actions to show or hide each entity type.
 * @return The created menu. It has no parent initially.
 */
QMenu* MainWindow::create_show_entities_menu() {

  QMenu* menu = new QMenu(tr("Show/hide entity types"));

  // Add show entity types actions to the menu.
  QList<QAction*> entity_actions = EnumMenus<EntityType>::create_actions(
        *menu,
        EnumMenuCheckableOption::CHECKABLE,
        [this](EntityType type) {
    Editor* editor = get_current_editor();
    if (editor != nullptr) {
      QString type_name = EntityTraits::get_lua_name(type);
      const bool visible = show_entities_subactions[type_name]->isChecked();
      editor->get_view_settings().set_entity_type_visible(type, visible);
    }
  });

  Q_FOREACH (QAction* action, entity_actions) {
    EntityType type = static_cast<EntityType>(action->data().toInt());
    if (!EntityTraits::can_be_stored_in_map_file(type)) {
      // Only show the ones that can exist in map files.
      menu->removeAction(action);
      continue;
    }
    QString type_name = EntityTraits::get_lua_name(type);
    show_entities_subactions[type_name] = action;
  }

  // Add special actions Show all and Hide all.
  QAction* show_all_action = new QAction(tr("Show all entities"), this);
  show_entities_subactions["action_show_all"] = show_all_action;
  menu->insertAction(entity_actions.first(), show_all_action);
  connect(show_all_action, &QAction::triggered, [this]() {
    Editor* editor = get_current_editor();
    if (editor != nullptr) {
      editor->get_view_settings().show_all_entity_types();
    }
  });

  QAction* hide_all_action = new QAction(tr("Hide all entities"), this);
  show_entities_subactions["action_hide_all"] = hide_all_action;
  menu->insertAction(entity_actions.first(), hide_all_action);
  connect(hide_all_action, &QAction::triggered, [this]() {
    Editor* editor = get_current_editor();
    if (editor != nullptr) {
      editor->get_view_settings().hide_all_entity_types();
    }
  });

  menu->insertSeparator(entity_actions.first());

  return menu;
}

/**
 * @brief Sets an appropriate size and centers the window on the screen having
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
 * @brief Closes the current quest if any and without confirmation.
 */
void MainWindow::close_quest() {

  ui.tab_widget->close_without_confirmation();

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

  bool success = false;

  try {
    // Load the requested quest.
    quest.set_root_path(quest_path);

    if (!quest.exists()) {
      throw EditorException(tr("No quest was found in directory\n'%1'").arg(quest_path));
    }
    quest.check_version();

    connect(&quest, SIGNAL(file_renamed(QString, QString)),
            ui.tab_widget, SLOT(file_renamed(QString, QString)));
    connect(&quest, SIGNAL(file_deleted(QString)),
            ui.tab_widget, SLOT(file_deleted(QString)));

    ui.action_run_quest->setEnabled(true);

    add_quest_to_recent_list();

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
        ui.action_run_quest->setEnabled(true);
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

  return success;
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
 * @brief Adds the quest path to the list of recent quests.
 *
 * Moves it to the beginning of the list if it is already presents.
 * Keeps the number of recent quests in the list limited.
 */
void MainWindow::add_quest_to_recent_list() {

  EditorSettings settings;
  QStringList last_quests = settings.get_value_string_list(EditorSettings::last_quests);
  QString quest_path = get_quest().get_root_path();

  if (!last_quests.isEmpty() && last_quests.first() == quest_path) {
    // Nothing to do.
    return;
  }

  // Remove if already present.
  if (last_quests.contains(quest_path)) {
    last_quests.removeAll(quest_path);
  }

  // Add to the beginning of the list.
  last_quests.prepend(quest_path);

  // Keep the list limited to 10 quests.
  constexpr int max = 10;
  while (last_quests.size() > max) {
    last_quests.removeAt(last_quests.size() - 1);
  }

  settings.set_value(EditorSettings::last_quests, last_quests);

  // Rebuild the recent quest menu.
  update_recent_quests_menu();
}

/**
 * @brief Slot called when the user triggers the "New quest" action.
 */
void MainWindow::on_action_new_quest_triggered() {

  // Check unsaved files but don't close them yet
  // in case the user cancels.
  if (!confirm_before_closing()) {
    return;
  }

  EditorSettings settings;

  QString quest_path = QFileDialog::getExistingDirectory(
        this,
        tr("Select quest directory"),
        settings.get_value_string(EditorSettings::working_directory),
        QFileDialog::ShowDirsOnly);

  if (quest_path.isEmpty()) {
    return;
  }

  close_quest();

  try {
    NewQuestBuilder::create_initial_quest_files(quest_path);
    if (open_quest(quest_path)) {
      // Open the quest properties editor initially.
      open_file(quest, quest.get_data_path());
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

  // Check unsaved files but don't close them yet
  // in case the user cancels.
  if (!confirm_before_closing()) {
    return;
  }

  // Ask the quest path.
  EditorSettings settings;
  QString quest_path = QFileDialog::getExistingDirectory(
        this,
        tr("Select quest directory"),
        settings.get_value_string(EditorSettings::working_directory),
        QFileDialog::ShowDirsOnly);

  if (quest_path.isEmpty()) {
    // Canceled.
    return;
  }

  close_quest();
  open_quest(quest_path);
}

/**
 * @brief Slot called when the user triggers the "Close quest" action.
 */
void MainWindow::on_action_close_quest_triggered() {

  // Check unsaved files.
  if (!confirm_before_closing()) {
    return;
  }

  close_quest();
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
 * @brief Slot called when the user triggers the "Save all" action.
 */
void MainWindow::on_action_save_all_triggered() {

  ui.tab_widget->save_all_files_requested();
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
 * @brief Slot called when the user triggers the "Close all" action.
 */
void MainWindow::on_action_close_all_triggered() {

  ui.tab_widget->close_all_files_requested();
}

/**
 * @brief Slot called when the user triggers the "Exit" action.
 */
void MainWindow::on_action_exit_triggered() {

  if (confirm_before_closing()) {
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
 * @brief Slot called when the user triggers the "Select all" action.
 */
void MainWindow::on_action_select_all_triggered() {

  Editor* editor = get_current_editor();
  if (editor != nullptr) {
    editor->select_all();
  }
}

/**
 * @brief Slot called when the user triggers the "Unselect all" action.
 */
void MainWindow::on_action_unselect_all_triggered() {

  Editor* editor = get_current_editor();
  if (editor != nullptr) {
    editor->unselect_all();
  }
}

/**
 * @brief Slot called when the user triggers the "Find" action.
 */
void MainWindow::on_action_find_triggered() {

  Editor* editor = get_current_editor();
  if (editor != nullptr) {
    editor->find();
  }
}

/**
 * @brief Slot called when the user triggers the "Run quest" action.
 */
void MainWindow::on_action_run_quest_triggered() {

  if (!quest_runner.is_started()) {

    if (ui.tab_widget->has_unsaved_files()) {

      EditorSettings settings;
      const QString& save_files = settings.get_value_string(EditorSettings::save_files_before_running);

      QMessageBox::StandardButton answer = QMessageBox::Yes;
      if (save_files == "yes") {
        answer = QMessageBox::Yes;
      }
      else if (save_files == "no") {
        answer = QMessageBox::No;
      }
      else {
        answer = QMessageBox::warning(
              this,
              tr("Files are modified"),
              tr("Do you want to save modifications before running the quest?"),
              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
              QMessageBox::Yes
              );
      }
      if (answer == QMessageBox::Cancel) {
        return;
      }

      if (answer == QMessageBox::Yes) {
        ui.tab_widget->save_all_files_requested();
      }
    }

    quest_runner.start(quest.get_root_path());

    // Automatically show the console when the quest starts.
    set_console_visible(true);
  }
  else {
    quest_runner.stop();
  }
  update_run_quest();
}

/**
 * @brief Slot called when the user triggers the "Stop music" action.
 */
void MainWindow::on_action_stop_music_triggered() {

  Sound::stop_music(get_quest());
}

/**
 * @brief Slot called when the user triggers the "Show grid" action.
 */
void MainWindow::on_action_show_grid_triggered() {

  Editor* editor = get_current_editor();
  if (editor == nullptr) {
    return;
  }

  editor->get_view_settings().set_grid_visible(ui.action_show_grid->isChecked());
}

/**
 * @brief Slot called when the user triggers the "Show console" action.
 */
void MainWindow::on_action_show_console_triggered() {

  // Show or hide the console.
  const bool show_console = ui.action_show_console->isChecked();
  set_console_visible(show_console);
}

/**
 * @brief Returns whether the execution console is shown.
 * @return @c true if the execution console is visible.
 */
bool MainWindow::is_console_visible() const {

  return ui.console_widget->isVisible();
}

/**
 * @brief Shows or hide the execution visible.
 * @param console_visible @c true to show the console.
 */
void MainWindow::set_console_visible(bool console_visible) {

  if (!console_visible &&
      ui.console_widget->height() < 16) {
    // Hiding a very small console: make sure it gets a decent size
    // when restored later.
    const int console_height = 100;
    ui.console_splitter->setSizes({ height() - console_height, console_height });
  }

  ui.console_widget->setVisible(console_visible);
}

/**
 * @brief Slot called when the user changes the grid size.
 */
void MainWindow::change_grid_size() {

  Editor* editor = get_current_editor();
  if (editor == nullptr) {
    return;
  }

  editor->get_view_settings().set_grid_size(grid_size->get_size());
}

/**
 * @brief Slot called when the user triggers the "Show low layer" action.
 */
void MainWindow::on_action_show_layer_0_triggered() {

  Editor* editor = get_current_editor();
  if (editor == nullptr) {
    return;
  }

  editor->get_view_settings().set_layer_visible(0, ui.action_show_layer_0->isChecked());
}

/**
 * @brief Slot called when the user triggers the "Show intermediate layer" action.
 */
void MainWindow::on_action_show_layer_1_triggered() {

  Editor* editor = get_current_editor();
  if (editor == nullptr) {
    return;
  }

  editor->get_view_settings().set_layer_visible(1, ui.action_show_layer_1->isChecked());
}

/**
 * @brief Slot called when the user triggers the "Show high layer" action.
 */
void MainWindow::on_action_show_layer_2_triggered() {

  Editor* editor = get_current_editor();
  if (editor == nullptr) {
    return;
  }

  editor->get_view_settings().set_layer_visible(2, ui.action_show_layer_2->isChecked());
}

/**
 * @brief Slot called when the user triggers the "Settings" action.
 */
void MainWindow::on_action_settings_triggered() {

  settings_dialog.exec();
}

/**
 * @brief Slot called when the user triggers the "Website" action.
 */
void MainWindow::on_action_website_triggered() {

  QDesktopServices::openUrl(QUrl("http://www.solarus-games.org/"));
}

/**
 * @brief Slot called when the user triggers the "Documentation" action.
 */
void MainWindow::on_action_doc_triggered() {

  QDesktopServices::openUrl(
        QUrl("http://www.solarus-games.org/doc/latest/index.html"));
}

/**
 * @brief Slot called when the current editor changes.
 * @param index Index of the new current editor, or -1 if no editor is active.
 */
void MainWindow::current_editor_changed(int index) {

  Q_UNUSED(index);

  Editor* editor = get_current_editor();
  const bool has_editor = editor != nullptr;
  ViewSettings& view_settings = editor->get_view_settings();

  // Set up toolbar buttons for this editor.
  ui.action_cut->setEnabled(has_editor);
  ui.action_copy->setEnabled(has_editor);
  ui.action_paste->setEnabled(has_editor);
  ui.action_close->setEnabled(has_editor);
  ui.action_close_all->setEnabled(has_editor);
  ui.action_save->setEnabled(has_editor);
  ui.action_save_all->setEnabled(has_editor);

  const bool select_all_supported = has_editor && editor->is_select_all_supported();
  ui.action_select_all->setEnabled(select_all_supported);

  const bool find_supported = has_editor && editor->is_find_supported();
  ui.action_find->setEnabled(find_supported);

  const bool zoom_supported = has_editor && editor->is_zoom_supported();
  zoom_menu->setEnabled(zoom_supported);
  zoom_button->setEnabled(zoom_supported);

  const bool grid_supported = has_editor && editor->is_grid_supported();
  ui.action_show_grid->setEnabled(grid_supported);
  if (!grid_supported) {
    ui.action_show_grid->setChecked(false);
  }
  grid_size->setEnabled(ui.action_show_grid->isChecked());

  update_layer_range();

  bool entity_type_visibility_supported =
      has_editor && editor->is_entity_type_visibility_supported();
  show_entities_menu->setEnabled(entity_type_visibility_supported);
  show_entities_button->setEnabled(entity_type_visibility_supported);

  if (has_editor) {

    connect(&view_settings, SIGNAL(zoom_changed(double)),
            this, SLOT(update_zoom()));
    update_zoom();

    connect(&view_settings, SIGNAL(grid_visibility_changed(bool)),
            this, SLOT(update_grid_visibility()));
    update_grid_visibility();
    connect(&view_settings, SIGNAL(grid_size_changed(QSize)),
            this, SLOT(update_grid_size()));
    update_grid_visibility();
    update_grid_size();

    connect(&view_settings, SIGNAL(layer_range_changed(int, int)),
            this, SLOT(update_layer_range()));
    connect(&view_settings, SIGNAL(layer_visibility_changed(int, bool)),
            this, SLOT(update_layer_visibility(int)));
    update_layers_visibility();

    connect(&view_settings, SIGNAL(entity_type_visibility_changed(EntityType, bool)),
            this, SLOT(update_entity_type_visibility(EntityType)));
    update_entity_types_visibility();

    editor->set_common_actions(common_actions);
  }
}

/**
 * @brief Slot called when the zoom of the current editor changes.
 */
void MainWindow::update_zoom() {

  Editor* editor = get_current_editor();
  if (editor == nullptr) {
    return;
  }

  double zoom = editor->get_view_settings().get_zoom();

  if (zoom_actions.contains(zoom)) {
    zoom_actions[zoom]->setChecked(true);
  }
}

/**
 * @brief Slot called when the grid of the current editor was just shown or hidden.
 */
void MainWindow::update_grid_visibility() {

  Editor* editor = get_current_editor();
  if (editor == nullptr) {
    return;
  }

  bool visible = editor->get_view_settings().is_grid_visible();

  ui.action_show_grid->setChecked(visible);
  grid_size->setEnabled(visible);
}

/**
 * @brief Slot called when the grid size of the current editor has just changed.
 */
void MainWindow::update_grid_size() {

  Editor* editor = get_current_editor();
  if (editor == nullptr) {
    return;
  }

  grid_size->set_size(editor->get_view_settings().get_grid_size());
}

/**
 * @brief Updates the number of layer visibility buttons.
 *
 * This function is called when the range of layers of the current editor changes.
 */
void MainWindow::update_layer_range() {

  Editor* editor = get_current_editor();
  const bool has_editor = editor != nullptr;

  int min_layer = 0;
  int max_layer = -1;

  if (has_editor) {
    editor->get_layers_supported(min_layer, max_layer);
    ViewSettings& view_settings = editor->get_view_settings();
    view_settings.set_layer_range(min_layer, max_layer);
  }

  ui.action_show_layer_0->setVisible(max_layer >= 0);
  ui.action_show_layer_1->setVisible(max_layer >= 1);
  ui.action_show_layer_2->setVisible(max_layer >= 2);
  show_layers_action->setVisible(min_layer < 0 || max_layer >= 3);
  show_layers_menu->setEnabled(min_layer < 0 || max_layer >= 3);
  update_show_layers_menu();
}

/**
 * @brief Slot called when a layer of the current editor was just shown or hidden.
 * @param layer The layer whose visibility has just changed.
 */
void MainWindow::update_layer_visibility(int layer) {

  const Editor* editor = get_current_editor();
  if (editor == nullptr) {
    return;
  }
  const ViewSettings& view_settings = editor->get_view_settings();
  const bool visible = view_settings.is_layer_visible(layer);

  int min_layer = 0;
  int max_layer = 0;
  editor->get_layers_supported(min_layer, max_layer);

  switch (layer) {

  case 0:
    ui.action_show_layer_0->setChecked(visible);
    break;

  case 1:
    ui.action_show_layer_1->setChecked(visible);
    break;

  case 2:
    ui.action_show_layer_2->setChecked(visible);
    break;

  default:
    break;
  }

  // Update the show layer menu.
  const int index = layer + 3 - min_layer;  // Skip "Show all", "Hide all" and the separator.
  QAction* action = show_layers_menu->actions().value(index);
  if (action == nullptr) {
    qCritical() << "Missing show layer action for layer " << layer << ": " << index;
    return;
  }
  action->setChecked(visible);
}

/**
 * @brief Slot called when the visibility of all layers should be updated to the GUI.
 */
void MainWindow::update_layers_visibility() {

  Editor* editor = get_current_editor();
  if (editor == nullptr) {
    return;
  }

  int min_layer = 0;
  int max_layer = 0;
  editor->get_layers_supported(min_layer, max_layer);

  ViewSettings& view_settings = editor->get_view_settings();
  ui.action_show_layer_0->setChecked(view_settings.is_layer_visible(0));
  ui.action_show_layer_1->setChecked(max_layer >= 1 && view_settings.is_layer_visible(1));
  ui.action_show_layer_2->setChecked(max_layer >= 2 && view_settings.is_layer_visible(2));

  const QList<QAction*>& actions = show_layers_menu->actions();
  for (int i = min_layer; i <= max_layer; ++i) {
    QAction* action = actions.value(i - min_layer);
    if (action != nullptr) {
      action->setVisible(view_settings.is_layer_visible(i));
    }
  }
}

/**
 * @brief Slot called when a entity type of the current editor was just shown or hidden.
 * @param entity_type The entity type whose visibility has just changed.
 */
void MainWindow::update_entity_type_visibility(EntityType entity_type) {

  Editor* editor = get_current_editor();
  if (editor == nullptr) {
    return;
  }
  ViewSettings& view_settings = editor->get_view_settings();
  bool visible = view_settings.is_entity_type_visible(entity_type);

  QString type_name = EntityTraits::get_lua_name(entity_type);
  const auto& it = show_entities_subactions.find(type_name);
  if (it == show_entities_subactions.end() || it.value() == nullptr) {
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
void MainWindow::update_entity_types_visibility() {

  Editor* editor = get_current_editor();
  if (editor == nullptr) {
    return;
  }

  ViewSettings& view_settings = editor->get_view_settings();
  Q_FOREACH (QAction* action, show_entities_subactions) {
    if (action == nullptr) {
      qCritical() << tr("Missing show entity type action");
      return;
    }
    if (action->data().isValid()) {  // Skip special actions.
      EntityType entity_type = static_cast<EntityType>(action->data().toInt());
      action->setChecked(view_settings.is_entity_type_visible(entity_type));
    }
  }
}

/**
 * @brief Slot called when the quest has just started or stopped.
 */
void MainWindow::update_run_quest() {

  if (quest_runner.is_started()) {
    ui.action_run_quest->setIcon(QIcon(":/images/icon_stop.png"));
    ui.action_run_quest->setToolTip(tr("Stop quest"));
  } else {
    ui.action_run_quest->setIcon(QIcon(":/images/icon_start.png"));
    ui.action_run_quest->setToolTip(tr("Run quest"));
  }
}

/**
 * @brief Slot called when the quest execution begins.
 */
void MainWindow::quest_running() {

  // Update the run quest action.
  update_run_quest();
}

/**
 * @brief Slot called when the quest execution is finished.
 */
void MainWindow::quest_finished() {

  // Update the run quest action.
  update_run_quest();
}

/**
 * @brief Slot called when a music is started or stopped.
 * @param music_id Id of the music currently playing
 * or an empty string.
 */
void MainWindow::current_music_changed(const QString& music_id) {

  if (music_id.isEmpty()) {
    ui.action_pause_music->setEnabled(false);
    ui.action_stop_music->setEnabled(false);
  }
  else {
    ui.action_pause_music->setEnabled(true);
    ui.action_stop_music->setEnabled(true);

  }
}

/**
 * @brief Reloads settings.
 */
void MainWindow::reload_settings() {

  ui.tab_widget->reload_settings();
}

/**
 * @brief Updates the title of the window from the current quest.
 */
void MainWindow::update_title() {

  QString version = SOLARUSEDITOR_VERSION;
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

  if (confirm_before_closing()) {
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
bool MainWindow::confirm_before_closing() {

  return ui.tab_widget->confirm_before_closing();
}

/**
 * @brief Slot called when the user wants to rename a file.
 *
 * The new file name will be prompted to the user.
 * It may or may not be a resource element file.
 *
 * @param quest The quest that holds this file.
 * @param path Path of the file to rename.
 */
void MainWindow::rename_file_requested(Quest& quest, const QString& path) {

  if (path.isEmpty()) {
    return;
  }

  if (path == quest.get_data_path()) {
    // We don't want to rename the data directory.
    return;
  }

  int editor_index = ui.tab_widget->find_editor(path);
  if (editor_index != -1) {
    // Don't rename a file that has unsaved changes.
    const Editor* editor = ui.tab_widget->get_editor(editor_index);
    if (editor != nullptr && editor->has_unsaved_changes()) {
      QMessageBox::warning(this,
                           tr("File modified"),
                           tr("This file is open and has unsaved changes.\nPlease save it or close it before renaming."));
      ui.tab_widget->show_editor(path);
      return;
    }
  }

  ResourceType resource_type;
  if (quest.is_resource_path(path, resource_type)) {
    // Don't rename built-in resource directories.
    return;
  }

  try {
    QuestResources& resources = quest.get_resources();
    QString element_id;
    if (quest.is_resource_element(path, resource_type, element_id)) {
      // Change the filename (and thereforce the id) of a resource element.

      QString resource_friendly_type_name_for_id = resources.get_friendly_name_for_id(resource_type);
      bool ok = false;
      QString new_id = QInputDialog::getText(
            this,
            tr("Rename resource"),
            tr("New id for %1 '%2':").arg(resource_friendly_type_name_for_id, element_id),
            QLineEdit::Normal,
            element_id,
            &ok);

      if (ok && new_id != element_id) {
        Quest::check_valid_file_name(new_id);
        quest.rename_resource_element(resource_type, element_id, new_id);
      }
    }
    else {
      // Rename a regular file or directory.
      bool ok = false;
      QString file_name = QFileInfo(path).fileName();
      QString new_file_name = QInputDialog::getText(
            this,
            tr("Rename file"),
            tr("New name for file '%1':").arg(file_name),
            QLineEdit::Normal,
            file_name,
            &ok);

      if (ok && new_file_name != file_name) {

        Quest::check_valid_file_name(file_name);
        QString new_path = QFileInfo(path).path() + '/' + new_file_name;
        quest.rename_file(path, new_path);
      }
    }
  }
  catch (const EditorException& ex) {
    ex.show_dialog();
  }

}

}
