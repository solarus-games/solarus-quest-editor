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
#include "gui/gui_tools.h"
#include "gui/new_resource_element_dialog.h"
#include "quest.h"

/**
 * @brief Creates a dialog for a new resource element.
 * @param resource_type The type of resource to create.
 * @param parent parent The parent widget or nullptr.
 */
NewResourceElementDialog::NewResourceElementDialog(
    ResourceType resource_type, QWidget* parent) :
  QDialog(parent),
  resource_type(resource_type) {

  ui.setupUi(this);
}

/**
 * @brief Returns the type of resource to be created.
 * @return The type of resource.
 */
ResourceType NewResourceElementDialog::get_resource_type() const {
  return resource_type;
}

/**
 * @brief Returns resource element id entered by the user.
 * @return The resource element id.
 */
QString NewResourceElementDialog::get_element_id() const {

  return ui.id_line_edit->text();
}

/**
 * @brief Returns resource element description entered by the user.
 * @return The description.
 */
QString NewResourceElementDialog::get_element_description() const {

  return ui.description_line_edit->text();
}

/**
 * @brief Closes the dialog unless the user tries to set invalid data.
 * @param result Result code of the dialog.
 */
void NewResourceElementDialog::done(int result) {

  if (result == QDialog::Accepted) {

    if (get_element_id().isEmpty()) {
      GuiTools::errorDialog("Empty resource id");
      return;
    }

    if (!Quest::is_valid_file_name(get_element_id())) {
      GuiTools::errorDialog("Invalid resource id");
      return;
    }

    if (get_element_description().isEmpty()) {
      GuiTools::errorDialog("Empty resource description");
      return;
    }
  }

  QDialog::done(result);
}
