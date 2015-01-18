/*
 * Copyright (C) 2014-2015 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_RESOURCE_MODEL_H
#define SOLARUSEDITOR_RESOURCE_MODEL_H

#include <solarus/ResourceType.h>
#include <QStandardItemModel>

using ResourceType = Solarus::ResourceType;

class Quest;
class QuestResources;

/**
 * @brief A tree model with resources elements of a type.
 */
class ResourceModel : public QStandardItemModel {
  Q_OBJECT

public:

  ResourceModel(Quest& quest, ResourceType resource_type, QObject* parent = nullptr);

  Quest& get_quest();
  QuestResources& get_resources();

  void add_special_value(
      const QString& id, const QString& text, int index);
  QModelIndex get_element_index(const QString& element_id) const;

private slots:

  void element_added(
      ResourceType type, const QString& id, const QString& description);
  void element_removed(
      ResourceType type, const QString& id);
  void element_renamed(
      ResourceType type, const QString& old_id, const QString& new_id);
  void element_description_changed(
      ResourceType type, const QString& id, const QString& new_description);

private:

  void add_element(const QString& element_id);
  void remove_element(const QString& element_id);
  QStandardItem* find_or_create_dir_item(
      QStandardItem& parent, const QString& dir_name);
  QStandardItem* create_dir_item(const QString& dir_name);
  QStandardItem* create_element_item(const QString& element_id);
  const QStandardItem* get_element_item(const QString& element_id) const;
  QStandardItem* get_element_item(const QString& element_id);

  Quest& quest;                   /**< The quest. */
  ResourceType resource_type;     /**< The resource type represented in the model. */
  std::map<QString, QStandardItem*>
      items;                      /**< Mapping of items from element ids. */

};

#endif
