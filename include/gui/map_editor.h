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
#ifndef SOLARUSEDITOR_MAP_EDITOR_H
#define SOLARUSEDITOR_MAP_EDITOR_H

#include "gui/editor.h"
#include "map_model.h"
#include "ui_map_editor.h"

class QStatusBar;
class QToolBar;

/**
 * \brief A widget to edit graphically a map file.
 */
class MapEditor : public Editor {
  Q_OBJECT

public:

  MapEditor(Quest& quest, const QString& path, QWidget* parent = nullptr);

  MapModel& get_model();
  MapView& get_map_view();

  void save() override;
  bool can_cut() const override;
  void cut() override;
  bool can_copy() const override;
  void copy() override;
  bool can_paste() const override;
  void paste() override;

private slots:

  void update();

  void update_map_id_field();
  void update_description_to_gui();
  void set_description_from_gui();
  void update_size_field();
  void change_size_requested();
  void update_world_field();
  void world_check_box_changed();
  void change_world_requested();
  void update_floor_field();
  void floor_check_box_changed();
  void change_floor_requested();
  void update_location_field();
  void change_location_requested();
  void update_tileset_field();
  void tileset_selector_activated();
  void open_tileset_requested();
  void update_music_field();
  void music_selector_activated();
  void update_tileset_view();
  void update_tileset_id();
  void tileset_selection_changed();
  void map_selection_changed();
  void update_status_bar();

  void move_entities_requested(const QList<EntityIndex>& indexes,
                               const QPoint& translation,
                               bool allow_merge_to_previous);
  void resize_entities_requested(const QMap<EntityIndex, QRect>& boxes,
                               bool allow_merge_to_previous);
  void add_entities_requested(AddableEntities& entities);
  void remove_entities_requested(const QList<EntityIndex>& indexes);

private:

  void build_entity_creation_toolbar();
  void build_status_bar();
  void entity_creation_button_triggered(EntityType type, bool checked);
  void uncheck_entity_creation_buttons();

  Ui::MapEditor ui;                         /**< The map editor widgets. */
  QString map_id;                           /**< Id of the map being edited. */
  MapModel* model;                          /**< Map model being edited. */
  QToolBar* entity_creation_toolbar;        /**< Toolbar allowing to add each type of entity. */
  QStatusBar* status_bar;                   /**< Status bar with information about the map view. */
  bool ignore_tileset_selection_changes;    /**< Blocks the tileset_selection_changed() slot. */

};

#endif
