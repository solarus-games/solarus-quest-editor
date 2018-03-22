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
#include "widgets/gui_tools.h"
#include "widgets/import_dialog.h"
#include "editor_settings.h"
#include <QFileDialog>

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

  ui.source_quest_tree_view->set_read_only(true);
  ui.source_quest_tree_view->set_opening_files_allowed(false);

  ui.destination_quest_field->setText(destination_quest.get_root_path());
  ui.destination_quest_tree_view->set_quest(destination_quest);
  ui.destination_quest_tree_view->set_opening_files_allowed(false);

  connect(ui.destination_quest_tree_view, SIGNAL(rename_file_requested(Quest&, QString)),
          this, SIGNAL(destination_quest_rename_file_requested(Quest&, QString)));
  connect(ui.source_quest_browse_button, SIGNAL(clicked(bool)),
          this, SLOT(browse_source_quest()));

  EditorSettings settings;
  QString last_source_quest_path = settings.get_value_string(EditorSettings::import_last_source_quest);

  connect(&source_quest, SIGNAL(root_path_changed(QString)),
          this, SLOT(source_quest_root_path_changed()));

  source_quest.set_root_path(last_source_quest_path);
  if (!source_quest.exists()) {
    browse_source_quest();
  }
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

/**
 * @brief Lets the user choose the source quest where to import from.
 */
void ImportDialog::browse_source_quest() {

  // Ask the quest path where to import from.
  EditorSettings settings;
  QString initial_value = settings.get_value_string(EditorSettings::import_last_source_quest);
  QString src_quest_path = QFileDialog::getExistingDirectory(
        this,
        tr("Select a quest where to import from"),
        initial_value,
        QFileDialog::ShowDirsOnly
  );

  if (src_quest_path.isEmpty()) {
    // Canceled.
    return;
  }

  if (src_quest_path == destination_quest.get_root_path()) {
    // Same quest.
    GuiTools::warning_dialog(tr("Source and destination quest are the same"));
    return;
  }

  source_quest.set_root_path(src_quest_path);
  if (!source_quest.exists()) {
    GuiTools::error_dialog(
          tr("No source quest was not found in directory '%1'").arg(src_quest_path));
  }
}

/**
 * @brief Slot called when the root path of the source quest has changed.
 */
void ImportDialog::source_quest_root_path_changed() {

  ui.source_quest_browse_field->setText(source_quest.get_root_path());
  ui.source_quest_tree_view->set_quest(source_quest);

  ui.import_button->setEnabled(source_quest.exists());
  if (source_quest.exists()) {
    EditorSettings settings;
    settings.set_value(EditorSettings::import_last_source_quest, source_quest.get_root_path());
  }
}

}
