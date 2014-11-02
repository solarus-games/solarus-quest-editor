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
#ifndef SOLARUSEDITOR_QUEST_PROPERTIES_H
#define SOLARUSEDITOR_QUEST_PROPERTIES_H

#include <solarus/QuestProperties.h>
#include <QObject>

class Quest;

/**
 * @brief Stores the properties of a quest and sends signals when they change.
 */
class QuestProperties : public QObject {
  Q_OBJECT

public:

  explicit QuestProperties(Quest& quest);

  void save() const;

  QString get_solarus_version() const;

signals:

  // TODO

private slots:

  void reload();

private:

  Quest& quest;                                  /**< The quest. */
  Solarus::QuestProperties properties;           /**< The wrapped data. */

};

#endif
