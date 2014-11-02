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
#include "file_tools.h"
#include "new_quest_builder.h"
#include "quest.h"

namespace NewQuestBuilder {

/**
 * @brief Creates initial files of a new quest.
 * @param quest_path Root path of the quest to create.
 * The data directory will be created there.
 * @throws EditorException if the files creation failed.
 */
void create_initial_quest_files(const QString& quest_path) {

  // Create files from resources.
  FileTools::copy_recursive(":/initial_quest/data", quest_path + "/data");

  // Make sure all resource directories exist.
  Quest quest(quest_path);
  for (const auto& kvp : Solarus::QuestResources::get_resource_type_names()) {
    ResourceType resource_type = kvp.first;
    quest.create_dir_if_not_exists(quest.get_resource_path(resource_type));
  }
}

}
