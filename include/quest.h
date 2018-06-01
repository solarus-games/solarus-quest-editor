/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
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

#include <quest_database.h>
#include <quest_properties.h>
#include <solarus/core/ResourceType.h>
#include <QObject>
#include <QSet>

class QRegularExpression;

namespace SolarusEditor {

class TilesetModel;

/**
 * @brief A Solarus project that can be open with the editor.
 *
 * This class handles the various paths of all files in the project.
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

  void check_version() const;

  const QuestProperties& get_properties() const;
  QuestProperties& get_properties();

  const QuestDatabase& get_database() const;
  QuestDatabase& get_database();

  // Get paths.
  QString get_name() const;
  QString get_data_path() const;
  QString get_path_relative_to_data_path(const QString& path);
  QString get_properties_path() const;
  QString get_main_script_path() const;
  QString get_resource_list_path() const;
  QString get_resource_path(ResourceType resource_type) const;
  QString get_resource_element_path(ResourceType resource_type,
                                    const QString& element_id) const;
  QStringList get_resource_element_paths(ResourceType resource_type,
                                         const QString& element_id) const;
  QString get_dialogs_path(const QString& language_id) const;
  QString get_enemy_script_path(const QString& enemy_id) const;
  QString get_entity_script_path(const QString& custom_entity_id) const;
  QString get_font_path(const QString& font_id) const;
  QString get_item_script_path(const QString& item_id) const;
  QString get_language_path(const QString& language_id) const;
  QString get_language_images_path(const QString& language_id) const;
  QString get_language_text_path(const QString& language_id) const;
  QString get_map_data_file_path(const QString& map_id) const;
  QString get_map_script_path(const QString& map_id) const;
  QString get_music_path(const QString& music_id) const;
  QString get_sound_path(const QString& sound_id) const;
  QString get_sprite_path(const QString& sprite_id) const;
  QString get_sprite_image_path(const QString& src_image) const;
  QString get_strings_path(const QString& language_id) const;
  QString get_tileset_data_file_path(const QString& tileset_id) const;
  QString get_tileset_tiles_image_path(const QString& tileset_id) const;
  QString get_tileset_entities_image_path(const QString& tileset_id) const;
  QString get_shader_path(const QString& shader_id) const;

  // Check path properties.
  static bool is_valid_file_name(const QString& file_name);
  static void check_valid_file_name(const QString& file_name);
  bool is_in_root_path(const QString& path) const;
  bool is_data_path(const QString& path) const;
  void check_is_in_root_path(const QString& path) const;
  bool exists(const QString& path) const;
  void check_exists(const QString& path) const;
  void check_not_exists(const QString& path) const;
  bool is_dir(const QString& path) const;
  void check_is_dir(const QString& path) const;
  void check_not_is_dir(const QString& path) const;
  bool is_script(const QString& path) const;
  void check_is_script(const QString& path) const;
  bool is_shader_code(const QString& path) const;
  bool is_data_file(const QString& path) const;
  bool is_image(const QString& path) const;
  bool is_properties_path(const QString& path) const;
  bool is_resource_list_path(const QString& path) const;
  bool is_resource_path(const QString& path, ResourceType& resource_type) const;
  bool is_in_resource_path(const QString& path, ResourceType& resource_type) const;
  bool is_potential_resource_element(
      const QString& path, ResourceType& resource_type, QString& element_id) const;
  bool is_resource_element(
      const QString& path, ResourceType& resource_type, QString& element_id) const;
  bool has_resource_element(
      const QString& path, ResourceType& resource_type) const;
  bool is_in_resource_element(
      const QString& path, ResourceType& resource_type, QString& element_id) const;
  bool is_map_script(const QString& path, QString& map_id) const;
  bool is_tileset_tiles_file(const QString& path, QString& tileset_id) const;
  bool is_tileset_entities_file(const QString& path, QString& tileset_id) const;
  bool is_dialogs_file(const QString& path, QString& language_id) const;
  bool is_strings_file(const QString& path, QString& language_id) const;
  bool is_language_image_file(const QString& path, QString& language_id) const;

  // Create, rename and delete paths.
  void create_file(const QString& path);
  void create_file_from_template(
      const QString& output_file_path,
      const QString& template_file_path,
      const QRegularExpression& pattern,
      const QString& replacement
  );
  bool create_file_if_not_exists(const QString& path);
  void create_script(const QString& path);
  bool create_script_if_not_exists(const QString& path);
  void create_map_data_file(const QString& map_id);
  bool create_map_data_file_if_not_exists(const QString& map_id);
  void create_map_script(const QString& map_id);
  bool create_map_script_if_not_exists(const QString& map_id);
  void create_item_script(const QString& item_id);
  bool create_item_script_if_not_exists(const QString& item_id);
  void create_enemy_script(const QString& enemy_id);
  bool create_enemy_script_if_not_exists(const QString& enemy_id);
  void create_entity_script(const QString& entity_id);
  bool create_entity_script_if_not_exists(const QString& entity_id);

  void create_dir(const QString& path);
  bool create_dir_if_not_exists(const QString& path);
  void create_dir(const QString& parent_path, const QString& dir_name);
  bool create_dir_if_not_exists(const QString& parent_path, const QString& dir_name);
  void create_resource_element(ResourceType resource_type,
                               const QString& element_id, const QString& description);
  void rename_file(const QString& old_path, const QString& new_path);
  bool rename_file_if_exists(const QString& old_path, const QString& new_path);
  void rename_resource_element(ResourceType resource_type,
                               const QString& old_id, const QString& new_id);
  void delete_file(const QString& path);
  bool delete_file_if_exists(const QString& path);
  void delete_dir(const QString& path);
  bool delete_dir_if_exists(const QString& path);
  void delete_dir_recursive(const QString& path);
  bool delete_dir_recursive_if_exists(const QString& path);
  void delete_resource_element(ResourceType resource_type,
                               const QString& element_id);

  QString get_current_music_id() const;
  void set_current_music_id(const QString& music_id);

  TilesetModel* get_tileset(const QString& tileset_id) const;
  void tileset_saved(const TilesetModel* tileset) const;

signals:

  void root_path_changed(const QString& root_path);
  void file_created(const QString& path);
  void file_renamed(const QString& old_path, const QString& new_path);
  void file_deleted(const QString& path);
  void current_music_changed(const QString& music_id);

private:

  QString root_path;               /**< Root path of this quest.
                                    * An empty string means no quest. */

  QuestProperties properties;      /**< Properties given in quest.dat. */
  QuestDatabase database;          /**< Resources and files declared in project_db.dat. */
  QString current_music_id;        /**< Id of the music currently playing if any. */

  mutable QMap<QString, TilesetModel*>
      tilesets;                    /** Cache of loaded tilesets. */
};

}

#endif
