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
    { Solarus::ResourceType::MAP,      tr("Map", "resource_type")           },
    { Solarus::ResourceType::TILESET,  tr("Tileset", "resource_type")       },
    { Solarus::ResourceType::SPRITE,   tr("Sprite", "resource_type")        },
    { Solarus::ResourceType::MUSIC,    tr("Music", "resource_type")         },
    { Solarus::ResourceType::SOUND,    tr("Sound", "resource_type")         },
    { Solarus::ResourceType::ITEM,     tr("Item", "resource_type")          },
    { Solarus::ResourceType::ENEMY,    tr("Enemy", "resource_type")         },
    { Solarus::ResourceType::ENTITY,   tr("Custom entity", "resource_type") },
    { Solarus::ResourceType::LANGUAGE, tr("Language", "resource_type")      },
    { Solarus::ResourceType::FONT,     tr("Font", "resource_type")          },
  };

  resource_type_friendly_names_for_id = {
    //: To be used with a specific element id like: Rename Map X.
    { Solarus::ResourceType::MAP,      tr("Map", "resource_element")           },
    { Solarus::ResourceType::TILESET,  tr("Tileset", "resource_element")       },
    { Solarus::ResourceType::SPRITE,   tr("Sprite", "resource_element")        },
    { Solarus::ResourceType::MUSIC,    tr("Music", "resource_element")         },
    { Solarus::ResourceType::SOUND,    tr("Sound", "resource_element")         },
    { Solarus::ResourceType::ITEM,     tr("Item", "resource_element")          },
    { Solarus::ResourceType::ENEMY,    tr("Enemy", "resource_element")         },
    { Solarus::ResourceType::ENTITY,   tr("Custom entity", "resource_element") },
    { Solarus::ResourceType::LANGUAGE, tr("Language", "resource_element")      },
    { Solarus::ResourceType::FONT,     tr("Font", "resource_element")          },
  };

  resource_type_directory_friendly_names = {
    { Solarus::ResourceType::MAP,      tr("Map folder")           },
    { Solarus::ResourceType::TILESET,  tr("Tileset folder")       },
    { Solarus::ResourceType::SPRITE,   tr("Sprite folder")        },
    { Solarus::ResourceType::MUSIC,    tr("Music folder")         },
    { Solarus::ResourceType::SOUND,    tr("Sound folder")         },
    { Solarus::ResourceType::ITEM,     tr("Item folder")          },
    { Solarus::ResourceType::ENEMY,    tr("Enemy folder")         },
    { Solarus::ResourceType::ENTITY,   tr("Custom entity folder") },
    { Solarus::ResourceType::LANGUAGE, tr("Language folder")      },
    { Solarus::ResourceType::FONT,     tr("Font folder")          },
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

  if (quest.is_valid()) {
    resources.load_from_file(quest.get_resource_list_path().toStdString());
    // TODO throw an exception in case of error
  }
}

/**
 * @brief Saves the resource list to the project_db.dat file of the quest.
 * @throws EditorException If the save operation failed.
 */
void QuestResources::save() const {

  if (!quest.is_valid()) {
    throw EditorException("No quest");
  }

  QString file_name = quest.get_resource_list_path();
  QFile file(file_name);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    throw EditorException("Cannot open file '" + file_name + "' for writing");
  }

  QTextStream out(&file);
  out.setCodec("UTF-8");

  // Save each resource.
  const std::map<ResourceType, std::string> resource_type_names =
      Solarus::QuestResources::get_resource_type_names();
  for (const auto& kvp: resource_type_names) {

    const Solarus::QuestResources::ResourceMap& resource =
        resources.get_elements(kvp.first);
    for (const auto& element: resource) {

      const std::string& id = element.first;
      const std::string& description = element.second;

      out << QString::fromStdString(kvp.second)
          << "{ id = \""
          << QString::fromStdString(id.c_str())
          << "\", description = \""
          << QString::fromStdString(description.c_str())
          << "\" }\n";
    }
    out << "\n";
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
