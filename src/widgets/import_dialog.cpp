/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
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
#include "include/widgets/import_dialog.h"

namespace SolarusEditor {

/**
 * @brief Creates an import dialog.
 * @param destination_quest The destination quest to import files to.
 * It must exist.
 * @param parent Parent object or nullptr.
 */
ImportDialog::ImportDialog(Quest& destination_quest, QWidget* parent) :
  QDialog(parent),
  ui(),
  source_quest(),
  destination_quest(destination_quest) {

  ui.setupUi(this);

  Q_ASSERT(destination_quest.exists());
}

/**
 * @brief Returns the source quest where to import files from.
 * @return The source quest or an invalid quest if it was not set.
 */
const Quest& ImportDialog::get_source_quest() const {
  return source_quest;
}

/**
 * @brief Returns the destination quest where to import files to.
 * @return The destination quest.
 */
Quest& ImportDialog::get_destination_quest() const {
  return destination_quest;
}

/* TODO
  // Ask the quest path where to import from.
  EditorSettings settings;
  QString src_quest_path = QFileDialog::getExistingDirectory(
        this,
        tr("Select a quest where to import from"),
        settings.get_value_string(EditorSettings::working_directory),
        QFileDialog::ShowDirsOnly);

  if (src_quest_path.isEmpty()) {
    // Canceled.
    return;
  }

  if (src_quest_path == quest.get_root_path()) {
    // Same quest.
    GuiTools::warning_dialog(tr("Source and destination quest are the same"));
    return;
  }

  Quest src_quest(src_quest_path);
  if (!src_quest.exists()) {
    GuiTools::error_dialog(
          tr("No source quest was not found in directory '%1'").arg(src_quest_path));
    return;
  }
*/

}
