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

namespace {

/**
 * @brief Directory name of each resource type, relative to the quest data diretory.
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

  return exists(get_data_path() + "/quest.dat");
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
 *
 * For languages, the path returned is a directory.
 *
 * @param resource_type A Solarus quest resource type.
 * @param element_id A resource element id.
 * @return The path to the main file of this resource element.
 */
QString Quest::get_resource_element_path(ResourceType resource_type,
                                         const QString& element_id) const {

  return get_resource_element_paths(resource_type, element_id).first();
}

/**
 * @brief Returns the paths to the files of a resource element.
 *
 * Indeed, some resource types have several files.
 * For languages, only one path is returned: the language directory.
 *
 * @param resource_type A Solarus quest resource type.
 * @param element_id A resource element id.
 * @return The paths to the files of this resource element.
 * The first element of the list is considered as the main file.
 */
QStringList Quest::get_resource_element_paths(ResourceType resource_type,
                                              const QString& element_id) const {
  QStringList paths;
  switch (resource_type) {

  case ResourceType::LANGUAGE:
    paths << get_language_path(element_id);
    break;

  case ResourceType::MAP:
    paths << get_map_data_file_path(element_id)
          << get_map_script_path(element_id);
    break;

  case ResourceType::TILESET:
    paths << get_tileset_data_file_path(element_id)
          << get_tileset_tiles_image_path(element_id)
          << get_tileset_entities_image_path(element_id);
    break;

  case ResourceType::SPRITE:
    paths << get_sprite_path(element_id);
    break;

  case ResourceType::MUSIC:
    paths << get_music_path(element_id);
    break;

  case ResourceType::SOUND:
    paths << get_sound_path(element_id);
    break;

  case ResourceType::ITEM:
    paths << get_item_script_path(element_id);
    break;

  case ResourceType::ENEMY:
    paths << get_enemy_script_path(element_id);
    break;

  case ResourceType::ENTITY:
    paths << get_entity_script_path(element_id);
    break;

  case ResourceType::FONT:
    paths << get_font_path(element_id);
    break;
  }

  return paths;
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
 * @brief Returns the path to the images directory of a language.
 * @param language_id Id of a language.
 * @return The path to the images directory of this language.
 */
QString Quest::get_language_images_path(const QString& language_id) const {

  return get_language_path(language_id) + "/images";
}

/**
 * @brief Returns the path to the text directory of a language.
 * @param language_id Id of a language.
 * @return The path to the text directory of this language.
 */
QString Quest::get_language_text_path(const QString& language_id) const {

  return get_language_path(language_id) + "/text";
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
 * @param[in] path The path to test.
 * @param[out] resource_type The resource type found if any.
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
 * @param[in] path The path to test.
 * @param[out] resource_type The resource type found if any.
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
 * @brief Returns whether a string is a valid file name for creation.
 *
 * You should call this function to check the name before creating or renaming
 * a file.
 * Note: slashes are allowed but ".." sequences are not.
 *
 * @param name The name to test (assumed without a full path).
 * @return @c true if this is a valid file name.
 */
bool Quest::is_valid_file_name(const QString& name) {

  if (
      name.isEmpty() ||                // The file name should not be empty.
      name.contains('\\') ||           // The path separator should be '/'.
      name == "." ||                   // Current directory.
      name == ".." ||                  // Don't go up in the file hierarchy.
      name.startsWith("../") ||
      name.endsWith("/..") ||
      name.contains("/../") ||
      name.trimmed() != name           // The file name should not begin or
                                       // end with whitespaces.
      ) {
    return false;
  }

  // This does not mean it is okay, but we have at least eliminated some
  // nasty situations.
  return true;
}

/**
 * @brief Checks that a string is a valid file name for creation.
 *
 * You should call this function to check the name before creating or renaming
 * a file.
 * Note: slashes are allowed but ".." sequences are not.
 *
 * @param name The name to test (assumed without a full path).
 * @throws EditorException If this is not a valid file name.
 */
void Quest::check_valid_file_name(const QString& name) {

  if (!is_valid_file_name(name)) {
    if (name.isEmpty()) {
      throw EditorException(tr("Empty file name").arg(name));
    }
    else {
      throw EditorException(tr("Invalid file name: '%1'").arg(name));
    }
  }
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

  return exists(path) && QFileInfo(path).isDir();
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
 * @brief Attempts to create an empty file in this quest.
 * @param path Path of the file to create. It must not exist.
 * @throws EditorException In case of error.
 */
void Quest::create_file(const QString& path) {

  check_is_in_root_path(path);
  check_not_exists(path);

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
 * @return @c true if the file was created, @c false if it already existed.
 */
bool Quest::create_file_if_not_exists(const QString& path) {

  if (exists(path)) {
    check_not_is_dir(path);
    return false;
  }

  create_file(path);
  return true;
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
 * @return @c true if the file was created, @c false if it already existed.
 */
bool Quest::create_script_if_not_exists(const QString& path) {

  if (exists(path)) {
    check_is_script(path);
    return false;
  }

  create_script(path);
  return true;
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
  QString dir_name = QDir(path).dirName();
  check_valid_file_name(dir_name);
  if (!QDir(parent_path).mkdir(dir_name)) {
    throw EditorException(tr("Cannot create directory '%1'").arg(path));
  }
}

/**
 * @brief Attempts to delete a directory in this quest if it does not exist
 * yet.
 * @param path Path of the directory to create. If it already exists, it must
 * be a directory.
 * @throws EditorException In case of error.
 * @return @c true if the directory was created, @c false if it already existed.
 */
bool Quest::create_dir_if_not_exists(const QString& path) {

  if (exists(path)) {
    check_is_dir(path);
    return false;
  }

  create_dir(path);
  return true;
}

/**
 * @brief Attempts to create a directory in this quest.
 * @param parent_path Path of an existing directory.
 * @param dir_name Name of the new directory to create there. It must not exist.
 * @throws EditorException In case of error.
 */
void Quest::create_dir(const QString& parent_path, const QString& dir_name) {

  check_valid_file_name(dir_name);
  check_exists(parent_path);
  check_is_dir(parent_path);
  check_not_exists(parent_path + '/' + dir_name);

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
 * @return @c true if the directory was created, @c false if it already existed.
 */
bool Quest::create_dir_if_not_exists(const QString& parent_path, const QString& dir_name) {

  check_exists(parent_path);
  check_is_dir(parent_path);

  QString path = parent_path + '/' + dir_name;
  if (exists(path)) {
    check_is_dir(path);
    return false;
  }

  create_dir(parent_path, dir_name);
  return true;
}

/**
 * @brief Creates a resource element on filesystem and in the resource list.
 *
 * It is okay if a file for this element already exists in the filesystem.
 * Otherwise, it will be created if it is an editable type (map, tileset,
 * sprite, language...).
 * It is okay too if the resource element is already declared in the resource
 * list.
 * However, it is not okay if it is already present in both the filesystem and
 * the resource list.
 *
 * @param resource_type A type of resource.
 * @param element_id Id of the element to create.
 * @param description Description of the element to create.
 * @throws EditorException If an error occured.
 */
void Quest::create_resource_element(ResourceType resource_type,
                                    const QString& element_id, const QString& description) {

  Quest::check_valid_file_name(element_id);

  // Make sure the top-level directory of the resource type exists.
  create_dir_if_not_exists(get_resource_path(resource_type));

  bool done_on_filesystem = false;

  QStringList paths = get_resource_element_paths(resource_type, element_id);

  switch (resource_type) {

  case ResourceType::MAP:
  case ResourceType::ITEM:
  case ResourceType::SPRITE:
  case ResourceType::ENEMY:
  case ResourceType::ENTITY:
    // For these type of resources, files to create are simply blank text files.
    for (QString path: paths) {
      done_on_filesystem |= create_file_if_not_exists(path);
    }
    break;

  case ResourceType::TILESET:
    // Text file and two blank images.
    done_on_filesystem |= create_file_if_not_exists(get_tileset_data_file_path(element_id));
    // TODO create the two images
    break;

  case ResourceType::LANGUAGE:
    // Directory with several files.
    done_on_filesystem |= create_dir_if_not_exists(get_language_path(element_id));
    done_on_filesystem |= create_dir_if_not_exists(get_language_images_path(element_id));
    done_on_filesystem |= create_dir_if_not_exists(get_language_text_path(element_id));
    done_on_filesystem |= create_file_if_not_exists(get_dialogs_path(element_id));
    done_on_filesystem |= create_file_if_not_exists(get_strings_path(element_id));
    break;

  case ResourceType::MUSIC:
  case ResourceType::SOUND:
  case ResourceType::FONT:
    // We don't create any file for the user in these formats.
    break;

  }

  // Also declare it in the resource list.
  bool done_in_resource_list = false;
  if (!resources.exists(resource_type, element_id)) {
    done_in_resource_list = true;
    resources.add(resource_type, element_id, description);
    resources.save();
  }

  if (!done_on_filesystem && !done_in_resource_list) {
    // Nothing was added. This must be an error.
    throw EditorException("Nothing to create");
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
 * @return @c true if the file was renamed, @c false if the renaming was
 * already done.
 */
bool Quest::rename_file_if_exists(const QString& old_path, const QString& new_path) {

  if (!exists(old_path)) {
    // The old file does not exist anymore.
    check_exists(new_path);
    return false;
  }

  // Normal case: the old file still exists.
  check_not_exists(new_path);
  rename_file(old_path, new_path);
  return true;
}

/**
 * @brief Renames a resource element on the filesystem and in the resource list.
 *
 * It is okay if the renaming is already done in the filesystem.
 * It is okay too if the resource element has already the new id in the resource list.
 * However, it is not okay if it is already done in both.
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

  check_valid_file_name(new_id);

  if (resources.exists(resource_type, old_id) &&
      resources.exists(resource_type, new_id)) {
    throw EditorException(tr("A resource with id '%1' already exists").arg(new_id));
  }

  if (!resources.exists(resource_type, old_id) &&
      !resources.exists(resource_type, new_id)) {
    throw EditorException(tr("No such resource: '%1'").arg(old_id));
  }

  // Rename files from the filesystem.
  bool renamed_on_filesystem = false;
  QStringList old_paths = get_resource_element_paths(resource_type, old_id);
  QStringList new_paths = get_resource_element_paths(resource_type, new_id);
  for (int i = 0; i < old_paths.size(); ++i) {
    QString old_path = old_paths.at(i);
    QString new_path = new_paths.at(i);

    // Take care of not changing the extension for musics and fonts.
    QString extension = QFileInfo(old_path).suffix();
    QFileInfo new_path_info(new_path);
    if (new_path_info.suffix() != extension) {
      // For example when renaming music 'temple' to 'dungeon':
      // the old path was /some/quest/data/musics/temple.it
      // and the new path was initialized by default with
      // /some/quest/data/musics/dungeon.ogg
      new_path = new_path_info.path() + '/' +
          new_path_info.completeBaseName() + '/' + extension;
    }

    if (exists(old_path)) {
      renamed_on_filesystem = true;
      rename_file(old_path, new_path);
    }
  }

  // Also rename it in the resource list.
  bool renamed_in_resource_list = false;
  if (resources.exists(resource_type, old_id)) {
    renamed_in_resource_list = true;
    resources.rename(resource_type, old_id, new_id);
    resources.save();
  }

  if (!renamed_on_filesystem && !renamed_in_resource_list) {
    // Nothing was renamed. This must be an error.
    throw EditorException("Nothing to rename");
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
 * @return @c true if the file could be deleted, @c false if was already gone.
 */
bool Quest::delete_file_if_exists(const QString& path) {

  if (!exists(path)) {
    return false;
  }

  delete_file(path);
  return true;
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
 * @return @c true if the file could be deleted, @c false if was already gone.
 */
bool Quest::delete_dir_if_exists(const QString& path) {

  if (!exists(path)) {
    return false;
  }

  delete_dir(path);
  return true;
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
 * @return @c true if the file could be deleted, @c false if was already gone.
 */
bool Quest::delete_dir_recursive_if_exists(const QString& path) {

  if (!exists(path)) {
    return false;
  }

  delete_dir_recursive(path);
  return true;
}

/**
 * @brief Deletes a resource element from the filesystem and from the resource
 * list.
 *
 * It is okay if some of its files (or all its files) are already removed from
 * the filesystem.
 * It is okay too if the element is already gone from the resource list.
 * However, it is not okay if both all files are removed and the element is
 * gone from the resource list.
 *
 * @param resource_type A type of resource.
 * @param id Id of the element to remove.
 * @throws EditorException If an error occured.
 */
void Quest::delete_resource_element(
    ResourceType resource_type, const QString& element_id) {

  // Delete files from the filesystem.
  bool found_in_filesystem = false;
  QStringList paths = get_resource_element_paths(resource_type, element_id);
  for (const QString& path: paths) {
    if (is_dir(path)) {
      found_in_filesystem = true;
      delete_dir_recursive(path);
    }
    else if (exists(path)) {
      found_in_filesystem = true;
      delete_file(path);
    }
  }

  // Also remove it from the resource list.
  bool found_in_resource_list = false;
  if (resources.exists(resource_type, element_id)) {
    found_in_resource_list = true;
    resources.remove(resource_type, element_id);
    resources.save();
  }

  if (!found_in_filesystem && !found_in_resource_list) {
    // Nothing was done. This must be an error.
    throw EditorException(tr("Nothing to delete"));
  }
}
