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

/**
 * @brief Creates a quest manager.
 * @param parent The parent QObject or nullptr.
 */
QuestManager::QuestManager(QObject* parent) :
  QObject(parent) {
  setObjectName("quest_manager");
}

/**
 * @brief Returns the current quest.
 * @return The current quest. An invalid quest is returned if there is no
 * current quest.
 */
Quest& QuestManager::get_quest() {
  return quest;
}

/**
 * @brief Sets another quest as the current one.
 *
 * Emits current_quest_changed(), even if the new one was not found.
 *
 * @param quest_path The quest path to open.
 * @return @c true if the quest exists in this path.
 */
bool QuestManager::set_quest(const QString& quest_path) {

  quest.set_root_path(quest_path);
  emit current_quest_changed(quest);
  return quest.exists();
}
