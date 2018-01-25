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
#include "widgets/pattern_picker_dialog.h"
#include "tileset_model.h"

namespace SolarusEditor {

/**
 * @brief Creates a pattern picker dialog.
 * @param tileset The tileset to show.
 * @param parent Parent object or nullptr.
 */
PatternPickerDialog::PatternPickerDialog(
    TilesetModel& tileset,
    QWidget* parent) :
  QDialog(parent) {

  ui.setupUi(this);

  ui.tileset_view->set_model(&tileset);
  ui.tileset_view->set_read_only(true);
  // TODO disable multi-selection
  // TODO make sure that Return and Escape shortcuts act on the dialog
}

/**
 * @brief Returns the pattern picked by the user.
 * @return The pattern id or an empty string.
 */
QString PatternPickerDialog::get_pattern_id() const {

  TilesetModel* tileset = ui.tileset_view->get_model();
  if (tileset == nullptr) {
    return QString();
  }
  int pattern_index = tileset->get_selected_index();

  return tileset->index_to_id(pattern_index);
}

/**
 * @brief Closes the dialog.
 * @param result Result code of the dialog.
 */
void PatternPickerDialog::done(int result) {

  QDialog::done(result);
}

}
