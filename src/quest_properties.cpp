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
#include "quest_properties.h"

/**
 * @brief Creates quest properties for the specified quest.
 * @param quest The quest.
 */
QuestProperties::QuestProperties(Quest& quest) :
  quest(quest) {

  connect(&quest, SIGNAL(root_path_changed(const QString&)),
          this, SLOT(reload()));
  reload();
}

/**
 * @brief Reads quest.dat into this object.
 */
void QuestProperties::reload() {

  if (quest.is_valid()) {
    properties.import_from_file(quest.get_properties_path().toStdString());
    // TODO handle errors
  }
}

/**
 * @brief Saves the properties to the quest.dat file of the quest.
 * @throws EditorException If the save operation failed.
 */
void QuestProperties::save() const {

  if (!quest.is_valid()) {
    throw EditorException(tr("No quest"));
  }

  QString file_name = quest.get_properties_path();
  if (!properties.export_to_file(file_name.toStdString())) {
    throw EditorException(tr("Cannot write file '%1'").arg(file_name));
  }
}

/**
 * @brief Returns the Solarus compatibility version of the quest.
 * @return The Solarus version.
 */
QString QuestProperties::get_solarus_version() const {

  return QString::fromStdString(properties.get_solarus_version());
}
