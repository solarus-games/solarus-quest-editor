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
#include <QMap>
#include <QObject>

using ResourceType = Solarus::ResourceType;

class Quest;

/**
 * @brief Stores the resources of a quest and sends signals when they change.
 */
class QuestResources : public QObject {
  Q_OBJECT

public:

  explicit QuestResources(Quest& quest);

  void save() const;

  bool exists(ResourceType type, const QString& id) const;
  bool add(
      ResourceType resource_type,
      const QString& id,
      const QString& description
  );
  bool remove(
      ResourceType resource_type,
      const QString& id
  );
  bool rename(
      ResourceType resource_type,
      const QString& old_id,
      const QString& new_id
  );
  QString get_description(
      ResourceType type, const QString& id) const;
  bool set_description(
      ResourceType type, const QString& id, const QString& description);

  QString get_lua_name(ResourceType resource_type) const;
  QString get_friendly_name(ResourceType resource_type) const;
  QString get_friendly_name_for_id(ResourceType resource_type) const;
  QString get_directory_friendly_name(ResourceType resource_type) const;

signals:

  void element_added(
      ResourceType type, const QString& id, const QString& description);
  void element_removed(
      ResourceType type, const QString& id);
  void element_renamed(
      ResourceType type, const QString& old_id, const QString& new_id);
  void element_description_changed(
      ResourceType type, const QString& id, const QString& new_description);

private slots:

  void reload();

private:

  Quest& quest;                                  /**< The quest. */
  Solarus::QuestResources resources;             /**< The wrapped data. */

  QMap<ResourceType, QString>
      resource_type_friendly_names;              /**< Human-readable name of each resource type. */
  QMap<ResourceType, QString>
      resource_type_friendly_names_for_id;       /**< Human-readable name of each resource type,
                                                  * to be followed by an element id.
                                                  * For example the string "Tileset" in
                                                  * "Do you want to save Tileset 'House'?".
                                                  * This makes a difference in languages where a
                                                  * determiner is needed, like in French:
                                                  * "Voulez-vous sauvegarder le Tileset 'House'?" */
  QMap<ResourceType, QString>
      resource_type_directory_friendly_names;    /**< Human-readable name describing the directory
                                                  * of each resource type. */
};

#endif
