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
#include "gui/new_animation_dialog.h"
#include "gui/gui_tools.h"

/**
 * @brief Creates a new animation dialog.
 * @param parent Parent object or nullptr.
 */
NewAnimationDialog::NewAnimationDialog(QWidget* parent) :
  QDialog(parent) {

  ui.setupUi(this);

  ui.animation_name_field->selectAll();
}

/**
 * @brief Returns animation name entered by the user.
 * @return The animation name.
 */
QString NewAnimationDialog::get_animation_name() const {

  return ui.animation_name_field->text();
}

/**
 * @brief Sets the animation name displayed in the text edit.
 * @param value The value to set.
 */
void NewAnimationDialog::set_animation_name(const QString& animation_name) {

  ui.animation_name_field->setText(animation_name);
}

/**
 * @brief Closes the dialog unless the user tries to set invalid data.
 * @param result Result code of the dialog.
 */
void NewAnimationDialog::done(int result) {

  if (result == QDialog::Accepted) {

    if (get_animation_name().isEmpty()) {
      GuiTools::error_dialog("Empty animation name");
      return;
    }
  }

  QDialog::done(result);
}
