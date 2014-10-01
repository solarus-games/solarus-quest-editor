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
Quest::Quest() {
}

/**
 * @brief Creates a quest with the specified path.
 * @param root_path Root path of the quest.
 */
Quest::Quest(const QString& root_path):
  root_path() {
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
 * @param root_path The root path (above the data directory). An empty string
 * means an invalid quest.
 */
void Quest::set_root_path(const QString& root_path) {

  QFileInfo file_info(root_path);
  if (file_info.exists()) {
    this->root_path = file_info.canonicalFilePath();
  }
  else {
    this->root_path = root_path;
  }
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
 * @brief Returns the path to he main.lua script of this quest.
 * @return The path to the quest main script.
 * Returns an empty string if the quest is invalid.
 */
QString Quest::get_main_script_path() const {
  return get_data_path() + "/main.lua";
}

/**
 * @brief Returns the path to the directory of the specified resource type.
 * @param resource_type A Solarus quest resource type.
 * @return The path to the directory of this resource.
 */
QString Quest::get_resource_path(Solarus::ResourceType resource_type) const {

  auto it = resource_dirs.find(resource_type);
  if (it == resource_dirs.end()) {
    qWarning() << "Unknown resource type";
    return "";
  }

  const QString& dir_name = *it;
  return get_data_path() + '/' + dir_name;
}

/**
 * @brief Returns whether a path is a resource path.
 * @param path The path to test.
 * @param resource_type The resource type found if any.
 * @return \c true if this is a resource path.
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
 * @return \c true if this path is under a resource path.
 */
bool Quest::is_in_resource_path(const QString& path, Solarus::ResourceType& resource_type) const {

  for (auto it = resource_dirs.begin(); it != resource_dirs.end(); ++it) {
    if (path.startsWith(get_resource_path(it.key()))) {
      resource_type = it.key();
      return true;
    }
  }

  return false;
}

/**
 * @brief Returns whether a path is a resource element like a map, a tileset, etc.
 * @param path The path to test.
 * @param resource_type The resource type found if any.
 * @return \c true if this path is under a resource element.
 */
bool Quest::is_resource_element(const QString& path, Solarus::ResourceType& resource_type) const {

  if (!is_in_resource_path(path, resource_type)) {
    // We are not in a resource directory.
    return false;
  }

  // We are under a resource directory. Check if a resource element with this id is declared.
  QString resource_path = get_resource_path(resource_type);
  QString path_from_resource = path.right(path.size() - resource_path.size() - 1);
  QString element_id;
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
    // TODO
    ;
  }

  if (extensions.isEmpty()) {
    // TODO languages
    return true;
  }

  for (const QString& extension: extensions) {
    if (path_from_resource.endsWith(extension)) {
      element_id = path_from_resource.remove(path_from_resource.lastIndexOf(extension), extension.size());
      break;
    }
  }
  if (element_id.isEmpty()) {
    // Not an recognized extension.
    return false;
  }

  // TODO return quest.get_resource(resource_type).exists(element_id);
  return true;
}
