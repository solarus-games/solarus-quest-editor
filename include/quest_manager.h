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
#ifndef SOLARUSEDITOR_QUEST_MANAGER_H
#define SOLARUSEDITOR_QUEST_MANAGER_H

#include "quest.h"
#include <QObject>
#include <memory>

/**
 * @brief Stores the current quest and sends signals when it changes.
 *
 * Several quests objects can live at the same time, but this class
 * keeps the current one.
 */
class QuestManager : public QObject {
  Q_OBJECT

public:

  explicit QuestManager(QObject* parent = nullptr);

  Quest& get_quest();
  bool set_quest(const QString& quest_path);

signals:

  void current_quest_changed(Quest& quest);

private:

  Quest quest;   /**< The current quest, possibly an invalid one. */

};

#endif
