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
#include <QFile>
#include <QFileInfo>

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
 * @brief Returns the path of the data directory of the current quest.
 * @return The path to the quest data directory. Returns an empty string if
 * the quest is invalid.
 */
QString Quest::get_data_path() const {

  if (!is_valid()) {
    return "";
  }

  return get_root_path() + "/data";
}
