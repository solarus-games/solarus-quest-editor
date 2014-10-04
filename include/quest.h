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
#ifndef SOLARUSEDITOR_QUEST_H
#define SOLARUSEDITOR_QUEST_H

#include <quest_resources.h>
#include <solarus/ResourceType.h>
#include <QObject>

/**
 * @brief A Solarus project that can be open with the editor.
 */
class Quest: public QObject {
  Q_OBJECT

public:

  Quest();
  explicit Quest(const QString& root_path);

  QString get_root_path() const;
  void set_root_path(const QString& root_path);

  bool is_valid() const;
  bool exists() const;

  // Paths.
  QString get_name() const;
  QString get_data_path() const;
  QString get_main_script_path() const;
  QString get_resource_list_path() const;
  QString get_resource_path(Solarus::ResourceType resource_type) const;
  QString get_dialogs_path(const QString& language_id) const;
  QString get_enemy_script_path(const QString& enemy_id) const;
  QString get_entity_script_path(const QString& custom_entity_id) const;
  QString get_item_script_path(const QString& item_id) const;
  QString get_language_path(const QString& language_id) const;
  QString get_map_data_file_path(const QString& map_id) const;
  QString get_map_script_path(const QString& map_id) const;
  QString get_sprite_path(const QString& sprite_id) const;
  QString get_strings_path(const QString& language_id) const;
  QString get_tileset_path(const QString& tileset_id) const;

  bool is_in_root_path(const QString& path) const;
  bool is_resource_path(const QString& path, Solarus::ResourceType& resource_type) const;
  bool is_in_resource_path(const QString& path, Solarus::ResourceType& resource_type) const;
  bool is_resource_element(
      const QString& path, Solarus::ResourceType& resource_type, QString& element_id) const;

  // Resources.
  QuestResources& get_resources();

signals:

  void root_path_changed(const QString& root_path);

private:

  QString root_path;               /**< Root path of this quest.
                                    * An empty string means no quest. */

  QuestResources resources;   /**< Resources declared in project_db.dat. */
};

#endif
