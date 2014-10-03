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
#include "quest_resources.h"

/**
 * @brief Creates an empty resource list for the specified quest.
 * @param quest The quest.
 */
QuestResources::QuestResources(Quest& quest):
  quest(quest) {

  connect(&quest, SIGNAL(root_path_changed(const QString&)),
          this, SLOT(reload()));
  reload();
}

/**
 * @brief Reads project_db.dat and rebuilds the model.
 */
void QuestResources::reload() {

  resources.clear();

  if (quest.is_valid()) {
    resources.load_from_file(quest.get_resource_list_path().toStdString());
    // TODO throw an exception in case of error
  }
}

/**
 * @brief Returns whether a resource element exists.
 * @param type A type of resource.
 * @param id The id to lookup.
 * @return @c true if such an element exists in the resource.
 */
bool QuestResources::exists(ResourceType type, const QString& id) const {

  return resources.exists(type, id.toStdString());
}

/**
 * @brief Returns the description of a resource element.
 * @param type A type of resource.
 * @param id Id of the element to get.
 * @return The description of this element.
 * Returns an empty string if the element does not exist.
 */
QString QuestResources::get_description(
    ResourceType type, const QString& id) const {

  return QString::fromStdString(
        resources.get_description(type, id.toStdString()));
}

/**
 * @brief Changes the description of a resource element.
 * @param type A type of resource.
 * @param id Id of the element to change.
 * @param description The new description to set.
 */
void QuestResources::set_description(
    ResourceType type, const QString& id, const QString& description) {

  resources.set_description(type, id.toStdString(), description.toStdString());
  // TODO resources.save_to_file();
  emit element_description_changed(type, id, description);
}
