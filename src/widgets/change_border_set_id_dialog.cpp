/*
 * Copyright (C) 2014-2017 Christopho, Solarus - http://www.solarus-games.org
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
#include "widgets/change_border_set_id_dialog.h"
#include "widgets/gui_tools.h"
#include "tileset_model.h"

namespace SolarusEditor {

/**
 * @brief Creates a change border set id dialog.
 * @param initial_border_set_id Initial value of the border set id.
 * @param parent Parent object or nullptr.
 */
ChangeBorderSetIdDialog::ChangeBorderSetIdDialog(
    const QString& initial_border_set_id,
    QWidget* parent) :
  QDialog(parent) {

  ui.setupUi(this);

  ui.border_set_id_label->setText(tr("New id for border set '%1':").arg(initial_border_set_id));
  set_border_set_id(initial_border_set_id);
  ui.border_set_id_field->selectAll();
}

/**
 * @brief Returns the border set id entered by the user.
 * @return The border set id.
 */
QString ChangeBorderSetIdDialog::get_border_set_id() const {

  return ui.border_set_id_field->text();
}

/**
 * @brief Sets the border set id displayed in the text edit.
 * @param value The value to set.
 */
void ChangeBorderSetIdDialog::set_border_set_id(const QString& border_set_id) {

  ui.border_set_id_field->setText(border_set_id);
}

/**
 * @brief Closes the dialog unless the user tries to set invalid data.
 * @param result Result code of the dialog.
 */
void ChangeBorderSetIdDialog::done(int result) {

  if (result == QDialog::Accepted) {

    if (get_border_set_id().isEmpty()) {
      GuiTools::error_dialog("Empty border set id");
      return;
    }

    if (!TilesetModel::is_valid_border_set_id(get_border_set_id())) {
      GuiTools::error_dialog("Invalid border set id");
      return;
    }
  }

  QDialog::done(result);
}

} // namespace SolarusEditor
