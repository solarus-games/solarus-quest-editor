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
#include "gui/resource_model.h"
#include "quest.h"
#include "quest_resources.h"

/**
 * @brief Creates a resource model.
 * @param quest The quest.
 * @param resource_type Type of resources to show.
 * @param parent The parent object or nullptr.
 */
ResourceModel::ResourceModel(Quest& quest, ResourceType resource_type, QObject* parent) :
  QStandardItemModel(parent),
  quest(quest),
  resource_type(resource_type) {

  QStringList ids = get_resources().get_elements(this->resource_type);
  for (const QString& id : ids) {
    QString description = get_resources().get_description(resource_type, id);
    // TODO add an icon
    this->appendRow(new QStandardItem(description));
  }
}

/**
 * @brief Returns the quest.
 * @return The quest.
 */
Quest& ResourceModel::get_quest() {
  return quest;
}

/**
 * @brief Returns the resources of the quest.
 * @return The resources.
 */
QuestResources& ResourceModel::get_resources() {
  return quest.get_resources();
}
