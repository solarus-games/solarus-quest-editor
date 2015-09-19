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
#ifndef SOLARUSEDITOR_MAIN_WINDOW_H
#define SOLARUSEDITOR_MAIN_WINDOW_H

#include "quest.h"
#include "ui_main_window.h"
#include "gui/settings_dialog.h"
#include <solarus/entities/EntityType.h>
#include <QMainWindow>

class Editor;
class QToolButton;
class QuestRunner;
class PairSpinBox;

using EntityType = Solarus::EntityType;

/**
 * @brief Main window of the quest editor.
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

public:

  MainWindow(QWidget* parent);
  ~MainWindow();

  void initialize_geometry_on_screen();
  Quest& get_quest();
  void close_quest();
  bool open_quest(const QString& quest_path);
  void open_file(Quest& quest, const QString& path);
  Editor* get_current_editor();

private slots:

  // Menu actions.
  void on_action_new_quest_triggered();
  void on_action_load_quest_triggered();
  void on_action_save_triggered();
  void on_action_close_triggered();
  void on_action_exit_triggered();
  void on_action_cut_triggered();
  void on_action_copy_triggered();
  void on_action_paste_triggered();
  void on_action_select_all_triggered();
  void on_action_find_triggered();
  void on_action_run_quest_triggered();
  void on_action_show_grid_triggered();
  void change_grid_size();
  void on_action_show_layer_0_triggered();
  void on_action_show_layer_1_triggered();
  void on_action_show_layer_2_triggered();
  void on_action_settings_triggered();
  void on_action_website_triggered();
  void on_action_doc_triggered();

  void current_editor_changed(int index);
  void rename_file_requested(Quest& quest, const QString& path);
  void update_zoom();
  void update_grid_visibility();
  void update_grid_size();
  void update_layer_visibility(int layer);
  void update_layers_visibility();
  void update_entity_type_visibility(EntityType entity_type);
  void update_entity_types_visibility();

  void update_run_quest();
  void solarus_fatal(const QString& what);

  void reload_settings();

protected:

  void closeEvent(QCloseEvent* event) override;

private:

  bool confirm_close();
  void update_title();
  void upgrade_quest();
  QMenu* create_zoom_menu();
  QMenu* create_show_entities_menu();

  Ui::MainWindow ui;              /**< The main window widgets. */
  Quest quest;                    /**< The current quest open if any. */
  QuestRunner* quest_runner;      /**< The dedicated thread to run quest. */

  QMenu* zoom_menu;               /**< The zoom menu. */
  QToolButton* zoom_button;       /**< The zoom toolbar button. */
  QMap<double, QAction*>
      zoom_actions;               /**< Action of each zoom value. */
  PairSpinBox* grid_size;         /**< The grid size. */

  QMenu* show_entities_menu;      /**< The menu with the visibility of all entity types . */
  QToolButton*
      show_entities_button;       /**< The entity type visilibity toolbar button. */
  QMap<QString, QAction*>
      show_entities_actions;      /**< Actions in the show entity type menu.
                                   * There is one action for each entity type,
                                   * plus two special actions "Show all" and "Hide all".
                                   * The key is the entity type name or
                                   * "action_show_all" or "action_hide_all". */

  QMap<QString, QAction*>
      common_actions;             /**< Actions available to all editors. */

  SettingsDialog settings_dialog; /**< The settings dialog. */

};

#endif
