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
#include "editor_exception.h"
#include "quest.h"
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMap>

using ResourceType = Solarus::ResourceType;

namespace {

/**
 * Directory name of each resource type, relative to the quest data diretory.
 */
const QMap<ResourceType, QString> resource_dirs = {
  { ResourceType::MAP,      "maps"      },
  { ResourceType::TILESET,  "tilesets"  },
  { ResourceType::SPRITE,   "sprites"   },
  { ResourceType::MUSIC,    "musics"    },
  { ResourceType::SOUND,    "sounds"    },
  { ResourceType::ITEM,     "items"     },
  { ResourceType::ENEMY,    "enemies"   },
  { ResourceType::ENTITY,   "entities"  },
  { ResourceType::LANGUAGE, "languages" },
  { ResourceType::FONT,     "fonts"     },
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
QString Quest::get_resource_path(ResourceType resource_type) const {

  auto it = resource_dirs.find(resource_type);
  if (it == resource_dirs.end()) {
    qWarning() << tr("Unknown resource type");
    return "";
  }

  const QString& dir_name = *it;
  return get_data_path() + '/' + dir_name;
}

/**
 * @brief Returns the path to the main file of a resource element.
 * @param resource_type A Solarus quest resource type.
 * @param element_id A resource element id.
 * @return The path to the main file of this resource element.
 */
QString Quest::get_resource_element_path(ResourceType resource_type, const QString& element_id) const {

  // TODO get_resource_element_paths?
  switch (resource_type) {

  case ResourceType::LANGUAGE:
    return get_language_path(element_id);

  case ResourceType::MAP:
    return get_map_data_file_path(element_id);

  case ResourceType::TILESET:
    return get_tileset_data_file_path(element_id);

  case ResourceType::SPRITE:
    return get_sprite_path(element_id);

  case ResourceType::MUSIC:
    return get_music_path(element_id);

  case ResourceType::SOUND:
    return get_sound_path(element_id);

  case ResourceType::ITEM:
    return get_item_script_path(element_id);

  case ResourceType::ENEMY:
    return get_enemy_script_path(element_id);

  case ResourceType::ENTITY:
    return get_entity_script_path(element_id);

  case ResourceType::FONT:
    return get_font_path(element_id);
  }

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
 * @brief Returns the path to a font file.
 *
 * Several extensions are allowed for font.
 * The first existing file with an accepted extension is returned.
 * If such a file does not exist yet, the path corresponding to
 * the preferred extension is returned.
 *
 * @param font_id Id of a music.
 * @return The path to the font file.
 */
QString Quest::get_font_path(
    const QString& font_id) const {

  QString prefix = get_data_path() + "/fonts/" + font_id;
  QStringList extensions;
  extensions << ".ttf" << ".ttc" << ".fon";
  for (const QString& extension: extensions) {
    QString path = prefix + extension;
    if (QFileInfo(path).exists()) {
      return path;
    }
  }
  return prefix + extensions.first();
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
 * @brief Returns the path to a sound file.
 * @param sound_id Id of a sound.
 * @return The path to the sound file.
 */
QString Quest::get_sound_path(
    const QString& sound_id) const {

  return get_data_path() + "/sounds/" + sound_id + ".ogg";
}

/**
 * @brief Returns the path to a music file.
 *
 * Several extensions are allowed for musics.
 * The first existing file with an accepted extension is returned.
 * If such a file does not exist yet, the path corresponding to
 * the preferred extension is returned.
 *
 * @param music_id Id of a music.
 * @return The path to the music file.
 */
QString Quest::get_music_path(
    const QString& music_id) const {

  QString prefix = get_data_path() + "/musics/" + music_id;
  QStringList extensions;
  extensions << ".ogg" << ".it" << ".spc";
  for (const QString& extension: extensions) {
    QString path = prefix + extension;
    if (QFileInfo(path).exists()) {
      return path;
    }
  }
  return prefix + extensions.first();
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
QString Quest::get_tileset_data_file_path(
    const QString& tileset_id) const {

  return get_data_path() + "/tilesets/" + tileset_id + ".dat";
}

/**
 * @brief Returns the path to a tileset tiles image file.
 * @param tileset_id Id of a tileset.
 * @return The path to the tileset tiles image file.
 */
QString Quest::get_tileset_tiles_image_path(
    const QString& tileset_id) const {

  return get_data_path() + "/tilesets/" + tileset_id + ".tiles.png";
}

/**
 * @brief Returns the path to a tileset entities image file.
 * @param tileset_id Id of a tileset.
 * @return The path to the tileset entities image file.
 */
QString Quest::get_tileset_entities_image_path(
    const QString& tileset_id) const {

  return get_data_path() + "/tilesets/" + tileset_id + ".entities.png";
}

/**
 * @brief Returns whether a path is a resource path.
 * @param path The path to test.
 * @param resource_type The resource type found if any.
 * @return @c true if this is a resource path.
 */
bool Quest::is_resource_path(const QString& path, ResourceType& resource_type) const {

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
 * @return @c true if this path is under a resource path, even if it does not
 * exist yet.
 */
bool Quest::is_in_resource_path(const QString& path, ResourceType& resource_type) const {

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
 * @return @c true if this path is a resource element declared in the resource
 * list, even if its files do not exist yet.
 */
bool Quest::is_resource_element(
    const QString& path, ResourceType& resource_type, QString& element_id) const {

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
  case ResourceType::MAP:
  case ResourceType::TILESET:
  case ResourceType::SPRITE:
    extensions << ".dat";
    break;

  case ResourceType::MUSIC:
    extensions << ".ogg" << ".it" << ".spc";
    break;

  case ResourceType::SOUND:
    extensions << ".ogg";
    break;

  case ResourceType::ITEM:
  case ResourceType::ENEMY:
  case ResourceType::ENTITY:
    extensions << ".lua";
    break;

  case ResourceType::FONT:
    extensions << ".ttf" << ".ttc" << ".fon";
    break;

  case ResourceType::LANGUAGE:
    // No extension.
    break;
  }

  if (resource_type == ResourceType::LANGUAGE) {
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
 * @return @c true if this path is a map script, even if it does not exist yet.
 */
bool Quest::is_map_script(const QString& path, QString& map_id) const {

  QString maps_path = get_resource_path(ResourceType::MAP);
  if (!path.startsWith(maps_path + "/")) {
    // We are not in the maps directory.
    return false;
  }

  if (!is_script(path)) {
    // Not a .lua file.
    return false;
  }

  QString path_from_maps = path.right(path.size() - maps_path.size() - 1);

  // Remove the extension.
  map_id = path_from_maps.section('.', 0, -2);
  if (!resources.exists(ResourceType::MAP, map_id)) {
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
 * @param[out] language_id Language of the dialogs if it is a dialog file.
 * @return @c true if this path is a dialogs file, even if it does not exist yet.
 */
bool Quest::is_dialogs_file(const QString& path, QString& language_id) const {

  QString languages_path = get_resource_path(ResourceType::LANGUAGE);
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
  if (!resources.exists(ResourceType::LANGUAGE, language_id)) {
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
 * @param[out] language_id Language of the strings if it is a dialog file.
 * @return @c true if this path is a strings file, even if it does not exist yet.
 */
bool Quest::is_strings_file(const QString& path, QString& language_id) const {

  QString languages_path = get_resource_path(ResourceType::LANGUAGE);
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
  if (!resources.exists(ResourceType::LANGUAGE, language_id)) {
    // Language id not declared in the resource list.
    return false;
  }

  return true;
}

/**
 * @brief Returns this resources declared in this quest.
 * @return The resources.
 */
const QuestResources& Quest::get_resources() const {
  return resources;
}

/**
 * @brief Returns this resources declared in this quest.
 * @return The resources.
 */
QuestResources& Quest::get_resources() {
  return resources;
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
 * @brief Checks that a path is under the quest path.
 *
 * It is okay if such a file does not exist yet.
 *
 * @throws EditorException If the path is not in this quest path.
 */
void Quest::check_is_in_root_path(const QString& path) const {

  if (!is_in_root_path(path)) {
    throw EditorException(tr("File '%1' is not in this quest").arg(path));
  }
}

/**
 * @brief Returns whether a path refers to an existing file or directory
 * of this quest.
 * @param path The path to test.
 * @return @c true if this path exists and is in this quest.
 */
bool Quest::exists(const QString& path) const {

  return is_in_root_path(path) && QFileInfo(path).exists();
}

/**
 * @brief Checks that a path refers to an existing file or directory of
 * this quest.
 * @throws EditorException If the path does not exist or is outside the quest.
 */
void Quest::check_exists(const QString& path) const {

  if (!exists(path)) {
    throw EditorException(tr("File '%1' does not exist").arg(path));
  }
}

/**
 * @brief Checks that no file or directory exists in a path of this quest.
 * @throws EditorException If the path already exists or is outside the quest.
 */
void Quest::check_not_exists(const QString& path) const {

  check_is_in_root_path(path);

  if (exists(path)) {
    throw EditorException(tr("File '%1' already exists").arg(path));
  }
}

/**
 * @brief Returns whether a path exists and is a directory of this quest.
 * @param path The path to test.
 * @return @c true if this path exists and is in the quest.
 */
bool Quest::is_dir(const QString& path) const {

  return exists(path) && QFileInfo(path).exists();
}

/**
 * @brief Checks that a path exists and is a directory of this quest.
 * @throws EditorException If the path is not a directory.
 */
void Quest::check_is_dir(const QString& path) const {

  check_exists(path);

  if (!QFileInfo(path).isDir()) {
    throw EditorException(tr("File '%1' is not a directory").arg(path));
  }
}

/**
 * @brief Checks that a path exists and is not a directory.
 * @throws EditorException If the path is a directory.
 */
void Quest::check_not_is_dir(const QString& path) const {

  check_exists(path);

  if (QFileInfo(path).isDir()) {
    throw EditorException(tr("File '%1' is a directory").arg(path));
  }
}

/**
 * @brief Returns whether a path of this quest corresponds to a Lua script.
 * @param path The path to test.
 * @return @c true if this path ends with ".lua", even if it does not exist yet.
 */
bool Quest::is_script(const QString& path) const {

  return is_in_root_path(path) && path.endsWith(".lua");
}

/**
 * @brief Checks that a path of this quest corresponds to a Lua script.
 *
 * It is okay if the script does not exist yet.
 *
 * @throws EditorException If the path does not end with ".lua".
 */
void Quest::check_is_script(const QString& path) const {

  if (!is_script(path)) {
    QString file_name(QFileInfo(path).fileName());
    throw EditorException(tr("Wrong script name: '%1' (should end with '.lua')").arg(file_name));
  }
}

/**
 * @brief Attempts to create a directory in this quest.
 * @param path Path of the directory to create. It must not exist.
 * @throws EditorException In case of error.
 */
void Quest::create_dir(const QString& path) {

  check_is_in_root_path(path);

  QString parent_path(path + "/..");
  check_exists(parent_path);
  if (!QDir(parent_path).mkdir(QDir(path).dirName())) {
    throw EditorException(tr("Cannot create directory '%1'").arg(path));
  }
}

/**
 * @brief Attempts to delete a directory in this quest if it does not exist
 * yet.
 * @param path Path of the directory to create. If it already exists, it must
 * be a directory.
 * @throws EditorException In case of error.
 */
void Quest::create_dir_if_not_exists(const QString& path) {

  if (exists(path)) {
    check_is_dir(path);
  }
  else {
    create_dir(path);
  }
}

/**
 * @brief Attempts to create a directory in this quest.
 * @param parent_path Path of an existing directory.
 * @param dir_name Name of the new directory to create there. It must not exist.
 * @throws EditorException In case of error.
 */
void Quest::create_dir(const QString& parent_path, const QString& dir_name) {

  check_exists(parent_path);
  check_is_dir(parent_path);

  if (!QDir(parent_path).mkdir(dir_name)) {
    throw EditorException(tr("Cannot create directory '%1'").arg(dir_name));
  }
}

/**
 * @brief Attempts to create a directory in this quest if it does not exist
 * yet.
 * @param parent_path Path of an existing directory.
 * @param dir_name Name of the new directory to create there. If it already
 * exists, it must be a directory.
 * @throws EditorException In case of error.
 */
void Quest::create_dir_if_not_exists(const QString& parent_path, const QString& dir_name) {

  check_exists(parent_path);
  check_is_dir(parent_path);

  QString path = parent_path + '/' + dir_name;
  if (exists(path)) {
    check_is_dir(path);
  }
  else {
    create_dir(parent_path, dir_name);
  }
}

/**
 * @brief Attempts to create an empty file in this quest.
 * @param path Path of the file to create. It must not exist.
 * @throws EditorException In case of error.
 */
void Quest::create_file(const QString& path) {

  check_is_in_root_path(path);

  if (!QFile(path).open(QIODevice::WriteOnly)) {
    throw EditorException(tr("Cannot create file '%1'").arg(path));
  }
  emit file_created(path);
}

/**
 * @brief Attempts to create a file in this quest if it does not exist yet.
 * @param path Path of the file to create. If it already exists, it must not
 * be a directory.
 * @throws EditorException In case of error.
 */
void Quest::create_file_if_not_exists(const QString& path) {

  if (exists(path)) {
    check_not_is_dir(path);
  }
  else {
    create_file(path);
  }
}

/**
 * @brief Attempts to create an empty Lua script file in this quest.
 * @param path Path of the file to create. It must end with ".lua".
 * It must not exist.
 * @throws EditorException In case of error.
 */
void Quest::create_script(const QString& path) {

  // Check that the file name ends with ".lua" and create it as an empty file.
  check_is_script(path);
  create_file(path);
}

/**
 * @brief Attempts to create a file in this quest if it does not exist yet.
 * @param path Path of the file to create. If it already exists, it must not
 * be a directory.
 * @throws EditorException In case of error.
 */
void Quest::create_script_if_not_exists(const QString& path) {

  if (exists(path)) {
    check_is_script(path);
  }
  else {
    create_script(path);
  }
}

/**
 * @brief Attempts to rename a file or directory of this quest.
 * @param old_path Path of the file to rename. It must exist.
 * @param new_path The new path. It must not exist.
 * @throws EditorException In case of error.
 */
void Quest::rename_file(const QString& old_path, const QString& new_path) {

  check_exists(old_path);
  check_not_exists(new_path);

  if (!QFile(old_path).rename(new_path)) {
    throw EditorException(tr("Cannot rename file '%1'").arg(old_path));
  }
  emit file_renamed(old_path, new_path);
}

/**
 * @brief Attempts to rename a file or directory of this quest if it exists.
 * @param old_path Path of the file to rename. If it no longer exists, then
 * the new file must exist.
 * @param new_path The new path. If it already exists, then the old file must
 * no longer exist.
 * @throws EditorException In case of error.
 */
void Quest::rename_file_if_exists(const QString& old_path, const QString& new_path) {

  if (!exists(old_path)) {
    // The old file does not exist anymore.
    check_exists(new_path);
  }
  else {
    // Normal case: the old file still exists.
    check_not_exists(new_path);
    rename_file(old_path, new_path);
  }
}

/**
 * @brief Attempts to delete a file of this quest.
 * @param path Path of the file to delete. It must not be a directory.
 * @throws EditorException In case of error.
 */
void Quest::delete_file(const QString& path) {

  check_not_is_dir(path);

  if (!QFile(path).remove()) {
    throw EditorException(tr("Cannot delete file '%1'").arg(path));
  }
  emit file_deleted(path);
}

/**
 * @brief Attempts to delete a file of this quest if it exists.
 * @param path Path of the file to delete. If it exists, it must not be a
 * directory.
 * @throws EditorException In case of error.
 */
void Quest::delete_file_if_exists(const QString& path) {

  if (exists(path)) {
    delete_file(path);
  }
}

/**
 * @brief Attempts to delete an empty directory of this quest.
 * @param path Path of the empty directory to delete. It must be a directory.
 * @throws EditorException In case of error.
 */
void Quest::delete_dir(const QString& path) {

  check_is_dir(path);

  QString parent_path(path + "/..");
  if (!QDir(parent_path).rmdir(QDir(path).dirName())) {
    throw EditorException(tr("Cannot delete directory '%1'").arg(path));
  }
}

/**
 * @brief Attempts to delete an empty directory of this quest if it exists.
 * @param path Path of the empty directory to delete. If it exists, if must
 * be a directory.
 * @throws EditorException In case of error.
 */
void Quest::delete_dir_if_exists(const QString& path) {

  if (exists(path)) {
    delete_dir(path);
  }
}

/**
 * @brief Attempts to delete a directory of this quest and all its content.
 * @param path Path of the directory to delete. It must be a directory.
 * @throws EditorException In case of error.
 */
void Quest::delete_dir_recursive(const QString& path) {

  check_is_dir(path);

  if (!QDir(path).removeRecursively()) {
    throw EditorException(tr("Cannot delete directory '%1'").arg(path));
  }
}

/**
 * @brief Attempts to delete a directory of this quest and all its content if
 * it exists.
 * @param path Path of the directory to delete. If it exists, if must
 * be a directory.
 * @throws EditorException In case of error.
 */
void Quest::delete_dir_recursive_if_exists(const QString& path) {

  if (exists(path)) {
    delete_dir_recursive(path);
  }
}

/**
 * @brief Renames a resource element in filesystem and in the resource list.
 *
 * It is okay if the renaming is already done in the filesystem.
 * It is okay too if the resource element has already the new id in the resource list.
 *
 * @param resource_type A type of resource.
 * @param old_id Id of the element to rename.
 * @param new_id The new id to set.
 * @throws EditorException If an error occured.
 */
void Quest::rename_resource_element(
    ResourceType resource_type, const QString& old_id, const QString& new_id) {

  // Sanity checks.
  if (new_id == old_id) {
    throw EditorException(tr("Same source and destination id").arg(new_id));
  }

  if (resources.exists(resource_type, old_id) && resources.exists(resource_type, new_id)) {
    throw EditorException(tr("A resource with id '%1' already exists").arg(new_id));
  }

  if (!resources.exists(resource_type, old_id) && !resources.exists(resource_type, new_id)) {
    throw EditorException(tr("No such resource: '%1'").arg(old_id));
  }

  switch (resource_type) {

  case ResourceType::LANGUAGE:
    // A language is a directory.
    rename_file_if_exists(get_language_path(old_id), get_language_path(new_id));
    break;

  case ResourceType::MAP:
    // Rename the map data file.
    rename_file_if_exists(get_map_data_file_path(old_id), get_map_data_file_path(new_id));

    // Also rename the map script.
    rename_file_if_exists(get_map_script_path(old_id), get_map_script_path(new_id));
    break;

  case ResourceType::TILESET:
    // Rename the tileset data file.
    rename_file_if_exists(get_tileset_data_file_path(old_id), get_tileset_data_file_path(new_id));

    // Also delete the two tileset images.
    rename_file_if_exists(get_tileset_tiles_image_path(old_id), get_tileset_tiles_image_path(new_id));
    rename_file_if_exists(get_tileset_entities_image_path(old_id), get_tileset_entities_image_path(new_id));
    break;

  case ResourceType::SPRITE:
    rename_file_if_exists(get_sprite_path(old_id), get_sprite_path(new_id));
    break;

  case ResourceType::MUSIC:
    rename_file_if_exists(get_music_path(old_id), get_music_path(new_id));
    break;

  case ResourceType::SOUND:
    rename_file_if_exists(get_sound_path(old_id), get_sound_path(new_id));
    break;

  case ResourceType::ITEM:
    rename_file_if_exists(get_item_script_path(old_id), get_item_script_path(new_id));
    break;

  case ResourceType::ENEMY:
    rename_file_if_exists(get_enemy_script_path(old_id), get_enemy_script_path(new_id));
    break;

  case ResourceType::ENTITY:
    rename_file_if_exists(get_entity_script_path(old_id), get_entity_script_path(new_id));
    break;

  case ResourceType::FONT:
    rename_file_if_exists(get_font_path(old_id), get_font_path(new_id));
    break;
  }

  // The file was successfully renamed on the filesystem.
  // Also rename it in the resource list.
  if (resources.exists(resource_type, old_id)) {
    resources.rename(resource_type, old_id, new_id);
    resources.save();
  }
}

/**
 * @brief Deletes a resource element from the filesystem and from the resource list.
 *
 * It is okay if some of its files are already removed.
 * It is okay too if the resource element was already removed from the resource list.
 *
 * @param resource_type A type of resource.
 * @param id Id of the element to remove.
 * @throws EditorException If an error occured.
 */
void Quest::delete_resource_element(
    ResourceType resource_type, const QString& element_id) {

  switch (resource_type) {

  case ResourceType::LANGUAGE:
    // A language is a directory.
    delete_dir_recursive_if_exists(get_language_path(element_id));
    break;

  case ResourceType::MAP:
    // Remove the map data file.
    delete_file_if_exists(get_map_data_file_path(element_id));

    // Also delete the map script.
    delete_file_if_exists(get_map_script_path(element_id));
    break;

  case ResourceType::TILESET:
    // Remove the tileset data file.
    delete_file_if_exists(get_tileset_data_file_path(element_id));

    // Also delete the two tileset images.
    delete_file_if_exists(get_tileset_tiles_image_path(element_id));
    delete_file_if_exists(get_tileset_entities_image_path(element_id));
    break;

  case ResourceType::SPRITE:
    delete_file_if_exists(get_sprite_path(element_id));
    break;

  case ResourceType::MUSIC:
    delete_file_if_exists(get_music_path(element_id));
    break;

  case ResourceType::SOUND:
    delete_file_if_exists(get_sound_path(element_id));
    break;

  case ResourceType::ITEM:
    delete_file_if_exists(get_item_script_path(element_id));
    break;

  case ResourceType::ENEMY:
    delete_file_if_exists(get_enemy_script_path(element_id));
    break;

  case ResourceType::ENTITY:
    delete_file_if_exists(get_entity_script_path(element_id));
    break;

  case ResourceType::FONT:
    delete_file_if_exists(get_font_path(element_id));
    break;
  }

  // The file was successfully removed from the filesystem.
  // Also remove it from the resource list.
  if (resources.exists(resource_type, element_id)) {
    resources.remove(resource_type, element_id);
    resources.save();
  }
}
