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
#include "quest.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMap>

namespace {

/**
 * Directory name of each resource type, relative to the quest data diretory.
 */
const QMap<Solarus::ResourceType, QString> resource_dirs = {
  { Solarus::ResourceType::MAP,      "maps"      },
  { Solarus::ResourceType::TILESET,  "tilesets"  },
  { Solarus::ResourceType::SPRITE,   "sprites"   },
  { Solarus::ResourceType::MUSIC,    "musics"    },
  { Solarus::ResourceType::SOUND,    "sounds"    },
  { Solarus::ResourceType::ITEM,     "items"     },
  { Solarus::ResourceType::ENEMY,    "enemies"   },
  { Solarus::ResourceType::ENTITY,   "entities"  },
  { Solarus::ResourceType::LANGUAGE, "languages" },
  { Solarus::ResourceType::FONT,     "fonts"     },
};

}

/**
 * @brief Creates an invalid quest.
 */
Quest::Quest():
  root_path(),
  resources(*this) {
}

/**
 * @brief Creates a quest with the specified path.
 * @param root_path Root path of the quest.
 */
Quest::Quest(const QString& root_path):
  root_path(),
  resources(*this) {
  set_root_path(root_path);
}

/**
 * @brief Returns whether this quest object is initialized.
 * @return @c false if this is no quest.
 */
bool Quest::is_valid() const {
  return !root_path.isEmpty();
}

/**
 * @brief Returns whether a quest exists in the root path.
 *
 * A quest is considered to exist if it contains a data directory with a
 * quest.dat file in it.
 *
 * @return @c true if the quest exists.
 */
bool Quest::exists() const {

  if (!is_valid()) {
    return false;
  }

  return QFileInfo(get_data_path() + "/quest.dat").exists();
}

/**
 * @brief Returns the path of this quest.
 * @return The root path (above the data directory).
 */
QString Quest::get_root_path() const {
  return root_path;
}

/**
 * @brief Sets the path of this quest.
 * @param root_path The root path (above the data directory).
 * An empty string means an invalid quest.
 */
void Quest::set_root_path(const QString& root_path) {

  QFileInfo file_info(root_path);
  if (file_info.exists()) {
    this->root_path = file_info.canonicalFilePath();
  }
  else {
    this->root_path = root_path;
  }

  emit root_path_changed(root_path);
}

/**
 * @brief Returns the name of this quest.
 *
 * The name returned is the last component of the quest path.
 *
 * @return The name of the quest root directory.
 */
QString Quest::get_name() const {

  return get_root_path().section('/', -1, -1, QString::SectionSkipEmpty);
}

/**
 * @brief Returns the path of the data directory of this quest.
 * @return The path to the quest data directory.
 * Returns an empty string if the quest is invalid.
 */
QString Quest::get_data_path() const {

  if (!is_valid()) {
    return "";
  }

  return get_root_path() + "/data";
}

/**
 * @brief Returns the path to the main.lua script of this quest.
 * @return The path to the quest main script.
 * Returns an empty string if the quest is invalid.
 */
QString Quest::get_main_script_path() const {

  if (!is_valid()) {
    return "";
  }

  return get_data_path() + "/main.lua";
}

/**
 * @brief Returns the path to the project.dat resource file of this quest.
 * @return The path to the resource list file.
 * Returns an empty string if the quest is invalid.
 */
QString Quest::get_resource_list_path() const {

  if (!is_valid()) {
    return "";
  }

  return get_data_path() + "/project_db.dat";
}

/**
 * @brief Returns the path to the directory of the specified resource type.
 * @param resource_type A Solarus quest resource type.
 * @return The path to the directory of this resource.
 */
QString Quest::get_resource_path(Solarus::ResourceType resource_type) const {

  auto it = resource_dirs.find(resource_type);
  if (it == resource_dirs.end()) {
    qWarning() << tr("Unknown resource type");
    return "";
  }

  const QString& dir_name = *it;
  return get_data_path() + '/' + dir_name;
}

/**
 * @brief Returns the path to an enemy script file.
 * @param language_id Id of an enemy.
 * @return The path to the enemy script file.
 */
QString Quest::get_enemy_script_path(
    const QString& enemy_id) const {

  return get_data_path() + "/enemies/" + enemy_id + ".lua";
}

/**
 * @brief Returns the path to a custom entity script file.
 * @param custom_entity_id Id of an custom entity model.
 * @return The path to the custom entity script file.
 */
QString Quest::get_entity_script_path(
    const QString& custom_entity_id) const {

  return get_data_path() + "/entities/" + custom_entity_id + ".lua";
}

/**
 * @brief Returns the path to an item script file.
 * @param item_id Id of an item.
 * @return The path to the item script file.
 */
QString Quest::get_item_script_path(
    const QString& item_id) const {

  return get_data_path() + "/items/" + item_id + ".lua";
}

/**
 * @brief Returns the path to a language directory.
 * @param language_id Id of a language.
 * @return The path to the directory of this language.
 */
QString Quest::get_language_path(const QString& language_id) const {

  return get_data_path() + "/languages/" + language_id;
}

/**
 * @brief Returns the path to a map data file.
 * @param map_id Id of a map.
 * @return The path to the map data file.
 */
QString Quest::get_map_data_file_path(
    const QString& map_id) const {

  return get_data_path() + "/maps/" + map_id + ".dat";
}

/**
 * @brief Returns the path to a map script file.
 * @param map_id Id of a map.
 * @return The path to the map script file.
 */
QString Quest::get_map_script_path(
    const QString& map_id) const {

  return get_data_path() + "/maps/" + map_id + ".lua";
}

/**
 * @brief Returns the path to a sprite sheet file.
 * @param sprite_id Id of a sprite sheet.
 * @return The path to the sprite sheet file.
 */
QString Quest::get_sprite_path(
    const QString& sprite_id) const {

  return get_data_path() + "/sprites/" + sprite_id + ".dat";
}

/**
 * @brief Returns the path to a dialogs file.
 * @param language_id Id of a language.
 * @return The path to the dialogs file of this language.
 */
QString Quest::get_dialogs_path(
    const QString& language_id) const {

  return get_language_path(language_id) + "/text/dialogs.dat";
}

/**
 * @brief Returns the path to a strings file.
 * @param language_id Id of a language.
 * @return The path to the strings file of this language.
 */
QString Quest::get_strings_path(
    const QString& language_id) const {

  return get_language_path(language_id) + "/text/strings.dat";
}

/**
 * @brief Returns the path to a tileset data file.
 * @param tileset_id Id of a tileset.
 * @return The path to the tileset data file.
 */
QString Quest::get_tileset_path(
    const QString& tileset_id) const {

  return get_data_path() + "/tilesets/" + tileset_id + ".dat";
}

/**
 * @brief Returns whether a path is under the quest path.
 * @param path The path to test.
 * @return @c true if this path is in the quest.
 */
bool Quest::is_in_root_path(const QString& path) const {

  return path.startsWith(get_root_path());
}

/**
 * @brief Returns whether a path is a resource path.
 * @param path The path to test.
 * @param resource_type The resource type found if any.
 * @return @c true if this is a resource path.
 */
bool Quest::is_resource_path(const QString& path, Solarus::ResourceType& resource_type) const {

  for (auto it = resource_dirs.begin(); it != resource_dirs.end(); ++it) {
    if (path == get_resource_path(it.key())) {
      resource_type = it.key();
      return true;
    }
  }

  return false;
}

/**
 * @brief Returns whether a path is under a resource path.
 * @param path The path to test.
 * @param resource_type The resource type found if any.
 * @return @c true if this path is under a resource path.
 */
bool Quest::is_in_resource_path(const QString& path, Solarus::ResourceType& resource_type) const {

  for (auto it = resource_dirs.begin(); it != resource_dirs.end(); ++it) {
    if (path.startsWith(get_resource_path(it.key()) + "/")) {
      resource_type = it.key();
      return true;
    }
  }

  return false;
}

/**
 * @brief Determines if a path is a resource element like a map, a tileset, etc.
 * @param[in] path The path to test.
 * @param[out] resource_type The resource type found if any.
 * @param[out] element_id Id of the resource element if any.
 * @return @c true if this path is a resource element.
 */
bool Quest::is_resource_element(
    const QString& path, Solarus::ResourceType& resource_type, QString& element_id) const {

  if (!is_in_resource_path(path, resource_type)) {
    // We are not in a resource directory.
    return false;
  }

  if (is_resource_path(path, resource_type)) {
    // The top-level resource directory itself.
    return false;
  }

  // We are under a resource directory. Check if a resource element with this id is declared.
  QString resource_path = get_resource_path(resource_type);
  QString path_from_resource = path.right(path.size() - resource_path.size() - 1);
  QStringList extensions;
  switch (resource_type) {
  case Solarus::ResourceType::MAP:
  case Solarus::ResourceType::TILESET:
  case Solarus::ResourceType::SPRITE:
    extensions << ".dat";
    break;

  case Solarus::ResourceType::MUSIC:
    extensions << ".ogg" << ".it" << ".spc";
    break;

  case Solarus::ResourceType::SOUND:
    extensions << ".ogg";
    break;

  case Solarus::ResourceType::ITEM:
  case Solarus::ResourceType::ENEMY:
  case Solarus::ResourceType::ENTITY:
    extensions << ".lua";
    break;

  case Solarus::ResourceType::FONT:
    extensions << ".ttf" << ".ttc" << ".fon";
    break;

  case Solarus::ResourceType::LANGUAGE:
    // No extension.
    break;
  }

  if (resource_type == Solarus::ResourceType::LANGUAGE) {
    element_id = path_from_resource;
  }
  else {
    for (const QString& extension: extensions) {
      if (path_from_resource.endsWith(extension)) {
        // Remove the extension.
        element_id = path_from_resource.section('.', 0, -2);
        break;
      }
    }
  }

  if (element_id.isEmpty()) {
    // Not an recognized extension.
    return false;
  }

  if (!resources.exists(resource_type, element_id)) {
    // Valid id, but not declared in the resource list.
    return false;
  }

  return true;
}

/**
 * @brief Determines if a path is a map script.
 *
 * This function exists because the main map resource path (as recognized by
 * is_resource_element()) is the map data file, not the map script.
 *
 * @param[in] path The path to test.
 * @param[out] map_id Id of the map if it is a map script.
 * @return @c true if this path is a map script.
 */
bool Quest::is_map_script(const QString& path, QString& map_id) const {

  QString maps_path = get_resource_path(Solarus::ResourceType::MAP);
  if (!path.startsWith(maps_path + "/")) {
    // We are not in the maps directory.
    return false;
  }

  if (!path.endsWith(".lua")) {
    // Not a script.
    return false;
  }

  QString path_from_maps = path.right(path.size() - maps_path.size() - 1);

  // Remove the extension.
  map_id = path_from_maps.section('.', 0, -2);
  if (!resources.exists(Solarus::ResourceType::MAP, map_id)) {
    // Valid map id, but not declared in the resource list.
    return false;
  }

  return true;
}

/**
 * @brief Determines if a path is a language dialogs file.
 *
 * This function exists because the main language resource path (as recognized by
 * is_resource_element()) is the language directory, not the dialogs file.
 *
 * @param[in] path The path to test.
 * @param[out] map_id Id of the map if it is a map script.
 * @return @c true if this path is a map script.
 */
bool Quest::is_dialogs_file(const QString& path, QString& language_id) const {

  QString languages_path = get_resource_path(Solarus::ResourceType::LANGUAGE);
  if (!path.startsWith(languages_path + "/")) {
    // We are not in the languages directory.
    return false;
  }

  QString expected_path_end = "/text/dialogs.dat";
  if (!path.endsWith(expected_path_end)) {
    // Not a dialogs file.
    return false;
  }

  QString path_from_languages = path.right(path.size() - languages_path.size() - 1);

  // Remove "/text/dialogs.dat" to determine the language id.
  language_id = path_from_languages.left(path_from_languages.size() - expected_path_end.size());
  if (!resources.exists(Solarus::ResourceType::LANGUAGE, language_id)) {
    // Language id not declared in the resource list.
    return false;
  }

  return true;
}

/**
 * @brief Determines if a path is a language strings file.
 *
 * This function exists because the main language resource path (as recognized by
 * is_resource_element()) is the language directory, not the strings file.
 *
 * @param[in] path The path to test.
 * @param[out] map_id Id of the map if it is a map script.
 * @return @c true if this path is a map script.
 */
bool Quest::is_strings_file(const QString& path, QString& language_id) const {

  QString languages_path = get_resource_path(Solarus::ResourceType::LANGUAGE);
  if (!path.startsWith(languages_path + "/")) {
    // We are not in the languages directory.
    return false;
  }

  QString expected_path_end = "/text/strings.dat";
  if (!path.endsWith(expected_path_end)) {
    // Not a dialogs file.
    return false;
  }

  QString path_from_languages = path.right(path.size() - languages_path.size() - 1);

  // Remove "/text/strings.dat" to determine the language id.
  language_id = path_from_languages.left(path_from_languages.size() - expected_path_end.size());
  if (!resources.exists(Solarus::ResourceType::LANGUAGE, language_id)) {
    // Language id not declared in the resource list.
    return false;
  }

  return true;
}

/**
 * @brief Returns this resources declared in this quest.
 * @return The resources.
 */
QuestResources& Quest::get_resources() {
  return resources;
}
