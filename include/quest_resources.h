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
#ifndef SOLARUSEDITOR_QUEST_RESOURCES_H
#define SOLARUSEDITOR_QUEST_RESOURCES_H

#include <solarus/QuestResources.h>
#include <QObject>

class Quest;

/**
 * @brief Stores the resources of a quest and sends signals when they change.
 */
class QuestResources : public QObject {
  Q_OBJECT

public:

  using ResourceType = Solarus::ResourceType;

  explicit QuestResources(Quest& quest);

  bool exists(ResourceType type, const QString& id) const;
  QString get_description(
      ResourceType type, const QString& id) const;
  void set_description(
      ResourceType type, const QString& id, const QString& description);

  static QString get_lua_name(ResourceType resource_type);
  static QString get_friendly_name(ResourceType resource_type);

signals:

  void element_added(
      ResourceType type, const QString& id, const QString& description);
  void element_removed(
      ResourceType type, const QString& id);
  void element_id_changed(
      ResourceType type, const QString& old_id, const QString& new_id);
  void element_description_changed(
      ResourceType type, const QString& id, const QString& new_description);

private slots:

  void reload();

private:

  Quest& quest;                        /**< The quest. */
  Solarus::QuestResources resources;   /**< The wrapped data. */

};

#endif
