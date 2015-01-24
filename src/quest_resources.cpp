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
#include "editor_exception.h"
#include "quest.h"
#include "quest_resources.h"
#include <QFile>
#include <QTextStream>

/**
 * @brief Creates an empty resource list for the specified quest.
 * @param quest The quest.
 */
QuestResources::QuestResources(Quest& quest):
  quest(quest) {

  // Friendly names are set dynamically because they are translated.
  resource_type_friendly_names = {
    //: To describe the type of resource itself like: New Map.
    { ResourceType::MAP,      tr("Map", "resource_type")           },
    { ResourceType::TILESET,  tr("Tileset", "resource_type")       },
    { ResourceType::SPRITE,   tr("Sprite", "resource_type")        },
    { ResourceType::MUSIC,    tr("Music", "resource_type")         },
    { ResourceType::SOUND,    tr("Sound", "resource_type")         },
    { ResourceType::ITEM,     tr("Item", "resource_type")          },
    { ResourceType::ENEMY,    tr("Enemy", "resource_type")         },
    { ResourceType::ENTITY,   tr("Custom entity", "resource_type") },
    { ResourceType::LANGUAGE, tr("Language", "resource_type")      },
    { ResourceType::FONT,     tr("Font", "resource_type")          },
  };

  resource_type_friendly_names_for_id = {
    //: To be used with a specific element id like: Rename Map X.
    { ResourceType::MAP,      tr("Map", "resource_element")           },
    { ResourceType::TILESET,  tr("Tileset", "resource_element")       },
    { ResourceType::SPRITE,   tr("Sprite", "resource_element")        },
    { ResourceType::MUSIC,    tr("Music", "resource_element")         },
    { ResourceType::SOUND,    tr("Sound", "resource_element")         },
    { ResourceType::ITEM,     tr("Item", "resource_element")          },
    { ResourceType::ENEMY,    tr("Enemy", "resource_element")         },
    { ResourceType::ENTITY,   tr("Custom entity", "resource_element") },
    { ResourceType::LANGUAGE, tr("Language", "resource_element")      },
    { ResourceType::FONT,     tr("Font", "resource_element")          },
  };

  resource_type_directory_friendly_names = {
    { ResourceType::MAP,      tr("Map folder")           },
    { ResourceType::TILESET,  tr("Tileset folder")       },
    { ResourceType::SPRITE,   tr("Sprite folder")        },
    { ResourceType::MUSIC,    tr("Music folder")         },
    { ResourceType::SOUND,    tr("Sound folder")         },
    { ResourceType::ITEM,     tr("Item folder")          },
    { ResourceType::ENEMY,    tr("Enemy folder")         },
    { ResourceType::ENTITY,   tr("Custom entity folder") },
    { ResourceType::LANGUAGE, tr("Language folder")      },
    { ResourceType::FONT,     tr("Font folder")          },
  };

  connect(&quest, SIGNAL(root_path_changed(const QString&)),
          this, SLOT(reload()));
  reload();
}

/**
 * @brief Reads project_db.dat and rebuilds the model.
 */
void QuestResources::reload() {

  resources.clear();

  // TODO don't try this if the quest format is obsolete
  if (quest.is_valid()) {
    resources.import_from_file(quest.get_resource_list_path().toStdString());
    // TODO throw an exception in case of error
  }
}

/**
 * @brief Saves the resource list to the project_db.dat file of the quest.
 * @throws EditorException If the save operation failed.
 */
void QuestResources::save() const {

  if (!quest.is_valid()) {
    throw EditorException(tr("No quest"));
  }

  QString file_name = quest.get_resource_list_path();
  if (!resources.export_to_file(file_name.toStdString())) {
    throw EditorException(tr("Cannot write file '%1'").arg(file_name));
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
 * @brief Returns the ids of all elements of a resource type.
 * @param type A type of resource.
 * @return All ids declared for this resource type.
 */
QStringList QuestResources::get_elements(ResourceType type) const {

  QStringList ids;
  for (const auto& kvp : resources.get_elements(type)) {
    ids << QString::fromStdString(kvp.first);
  }

  return ids;
}

/**
 * @brief Adds a resource element to the list.
 * @param resource_type A type of resource.
 * @param id Id of the element to add.
 * @param description Description the element to add.
 * @return @c true if the element was added, @c false if an element with
 * this id already exists.
 */
bool QuestResources::add(
    ResourceType resource_type,
    const QString& id,
    const QString& description
) {

  if (!resources.add(resource_type, id.toStdString(), description.toStdString())) {
    return false;
  }
  emit element_added(resource_type, id, description);
  return true;
}

/**
 * @brief Removes a resource element from the list.
 * @param resource_type A type of resource.
 * @param id Id of the element to remove.
 * @return @c true if the element was removed, @c false if such an element
 * did not exist.
 */
bool QuestResources::remove(
    ResourceType resource_type,
    const QString& id
) {

  if (!resources.remove(resource_type, id.toStdString())) {
    return false;
  }

  emit element_removed(resource_type, id);
  return true;
}

/**
 * @brief Changes the id of a resource element from the list.
 * @param resource_ A type of resource.
 * @param old_id Id of the element to change.
 * @param new_id The new id to set.
 * @return @c true in case of success, @c false if the old id does not
 * exist or if the new id already exists.
 */
bool QuestResources::rename(
    ResourceType resource_type,
    const QString& old_id,
    const QString& new_id
) {

  if (!resources.rename(resource_type, old_id.toStdString(),
                        new_id.toStdString())) {
    return false;
  }
  emit element_renamed(resource_type, old_id, new_id);
  return true;
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
 * @return @c true in case of success, @c false if such an element does not
 * exist.
 */
bool QuestResources::set_description(
    ResourceType type, const QString& id, const QString& description) {

  if (!resources.set_description(type, id.toStdString(), description.toStdString())) {
    return false;
  }
  emit element_description_changed(type, id, description);
  return true;
}

/**
 * @brief Returns the Lua name for the specified resource type.
 * @param resource_type A type of resource.
 * @return The Lua name of this resource type.
 */
QString QuestResources::get_lua_name(ResourceType resource_type) const {
  return QString::fromStdString(
        Solarus::QuestResources::get_resource_type_name(resource_type));
}

/**
 * @brief Returns a user-friendly name for the specified resource type.
 * @param resource_type A type of resources.
 * @return The human-readable name of this resource type.
 */
QString QuestResources::get_friendly_name(ResourceType resource_type) const {
  return resource_type_friendly_names[resource_type];
}

/**
 * @brief Returns a resource type user-friendly name to be followed by an it.
 *
 * For example the string "Tileset" in
 * "Do you want to save Tileset 'House'?".
 * This is different from get_friendly_name() in languages where a
 * determiner is needed, like in French:
 * "Voulez-vous sauvegarder le Tileset 'House'?"
 *
 * @param resource_type A type of resource.
 * @return The human-readable name of this resource type when it needs to be
 * followed by a resource element id.
 */
QString QuestResources::get_friendly_name_for_id(ResourceType resource_type) const {
  return resource_type_friendly_names_for_id[resource_type];
}

/**
 * @brief Returns a user-friendly name describing the top-level directory of a
 * resource type.
 * @param resource_type A type of resources.
 * @return The human-readable description of the corresponding directory.
 */
QString QuestResources::get_directory_friendly_name(ResourceType resource_type) const {
  return resource_type_directory_friendly_names[resource_type];
}

