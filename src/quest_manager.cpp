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
#include "quest_manager.h"
#include <QFile>

/**
 * @brief Creates a quest manager.
 * @param parent The parent QObject or nullptr.
 */
QuestManager::QuestManager(QObject* parent) :
  QObject(parent) {
  setObjectName("quest_manager");
}

/**
 * @brief Returns the path of the current quest.
 * @return The current quest path (above the data directory).
 */
QString QuestManager::get_quest_path() {
  return quest_path;
}

/**
 * @brief Sets another quest as the current one.
 * @param quest_path The quest path to open.
 * @return @c true in case of success, @c false if no quest was found in this
 * directory (the quest path is left unchanged in this case).
 */
bool QuestManager::set_quest_path(QString quest_path) {

  bool success = QFile(quest_path + "/data/quest.dat").exists();

  if (success) {
    this->quest_path = quest_path;
    emit current_quest_changed(quest_path);
  }
  return success;
}

/**
 * @brief Returns the name of the quest directory.
 *
 * The name returned is the last component of the quest path.
 *
 * @return The name of the quest directory.
 */
QString QuestManager::get_quest_name() {

  int index = quest_path.lastIndexOf('/');
  if (index == -1) {
    return quest_path;
  }

  return quest_path.section('/', -1);
}
