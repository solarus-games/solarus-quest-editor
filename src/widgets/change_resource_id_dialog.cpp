/*
 * Copyright (C) 2014-2016 Christopho, Solarus - http://www.solarus-games.org
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
#include "widgets/change_resource_id_dialog.h"
#include "widgets/gui_tools.h"
#include "editor_exception.h"
#include "quest.h"

namespace SolarusEditor {

/**
 * @brief Creates a change resource id dialog.
 * @param quest The quest.
 * @param resource_type Type of resource of the element to change.
 * @param old_id Element id before the change.
 * @param parent Parent object or nullptr.
 */
ChangeResourceIdDialog::ChangeResourceIdDialog(
    Quest& quest,
    ResourceType resource_type,
    const QString& old_id,
    QWidget* parent) :
  QDialog(parent),
  ui() {

  ui.setupUi(this);

  QuestResources& resources = quest.get_resources();
  const QString& resource_friendly_type_name_for_id =
      resources.get_friendly_name_for_id(resource_type);

  ui.resource_id_label->setText(
        tr("New id for %1 '%2':").arg(resource_friendly_type_name_for_id, old_id));

  set_element_id(old_id);
  set_update_references(true);

  if (resource_type == ResourceType::MAP) {
    ui.update_references_checkbox->setText(tr("Update existing teletransporters leading to this map"));
  }
  else {
    ui.update_references_checkbox->hide();
    layout()->removeWidget(ui.update_references_checkbox);
    adjustSize();
  }

  ui.resource_id_field->selectAll();
}

/**
 * @brief Returns the resource element id entered in the text edit.
 * @return The id entered in the field.
 */
QString ChangeResourceIdDialog::get_element_id() {

  return ui.resource_id_field->text();
}

/**
 * @brief Sets the resource element id entered in the text edit.
 * @param element_id The id to put in the field.
 */
void ChangeResourceIdDialog::set_element_id(const QString& element_id) {

  ui.resource_id_field->setText(element_id);
}

/**
 * @brief Returns whether the update references checkbox is checked.
 * @return @c the update references value.
 */
bool ChangeResourceIdDialog::get_update_references() const {

  return ui.update_references_checkbox->isChecked();
}

/**
 * @brief Sets whether the update references checkbox is checked.
 * @param update_references the update references value.
 */
void ChangeResourceIdDialog::set_update_references(bool update_references) {

  ui.update_references_checkbox->setChecked(update_references);
}

/**
 * @brief Closes the dialog unless the user tries to set invalid data.
 * @param result Result code of the dialog.
 */
void ChangeResourceIdDialog::done(int result) {

  if (result == QDialog::Accepted) {

    const QString& new_id = get_element_id();
    if (new_id.isEmpty()) {
      GuiTools::error_dialog(tr("Empty resource element id"));
      return;
    }

    if (!Quest::is_valid_file_name(new_id)) {
      GuiTools::error_dialog(tr("Invalid resource element id"));
      return;
    }
  }

  QDialog::done(result);
}

} // namespace SolarusEditor
