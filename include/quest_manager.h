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

#include <QObject>

/**
 * @brief Stores the current quest and sends signals when it changes.
 */
class QuestManager : public QObject {
  Q_OBJECT

public:

  explicit QuestManager(QObject* parent = nullptr);

  QString get_quest_path() const;
  bool set_quest_path(QString quest_path);

  QString get_quest_name() const;
  QString get_quest_data_path() const;

signals:

  void current_quest_changed(QString quest_path);

  // TODO
  // resource_element_added
  // resource_element_removed
  // resource_element_moved
  // resource_element_renamed

private:

  QString quest_path;   /**< Path of the current quest. */

};

#endif
