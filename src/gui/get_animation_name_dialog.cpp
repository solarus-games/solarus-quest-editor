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
#include "gui/get_animation_name_dialog.h"
#include "gui/gui_tools.h"
#include "sprite_model.h"

/**
 * @brief Creates a new animation name dialog.
 * @param model The sprite model.
 * @param parent Parent object or nullptr.
 */
GetAnimationNameDialog::GetAnimationNameDialog(
    const SpriteModel& model, QWidget* parent) :
  QInputDialog(parent),
  model(model),
  animation_name("") {

  setWindowTitle(tr("New animation"));
  setup_ui();
}

/**
 * @brief Creates a change animation name dialog.
 * @param model The sprite model.
 * @param animation_name The animation name.
 * @param parent Parent object or nullptr.
 */
GetAnimationNameDialog::GetAnimationNameDialog(
    const SpriteModel& model, const QString& animation_name, QWidget* parent) :
  QInputDialog(parent),
  model(model),
  animation_name(animation_name) {

  setWindowTitle(tr("Change animation name"));
  setup_ui();
  set_animation_name(animation_name);
}

/**
 * @brief Returns animation name entered by the user.
 * @return The animation name.
 */
QString GetAnimationNameDialog::get_animation_name() const {

  return textValue();
}

/**
 * @brief Sets the animation name displayed in the text edit.
 * @param value The value to set.
 */
void GetAnimationNameDialog::set_animation_name(const QString& animation_name) {

  setTextValue(animation_name);
}

/**
 * @brief Closes the dialog unless the user tries to set invalid data.
 * @param result Result code of the dialog.
 */
void GetAnimationNameDialog::done(int result) {

  if (result == QDialog::Accepted) {

    QString new_animation_name = textValue();

    if (new_animation_name.isEmpty()) {
      GuiTools::error_dialog(tr("Empty animation name"));
      return;
    }

    if (animation_name != new_animation_name &&
        model.animation_exists(new_animation_name)) {
      GuiTools::error_dialog(
            tr("Animation '%1' already exists").arg(new_animation_name));
      return;
    }
  }

  QDialog::done(result);
}

/**
 * @brief Setups the ui.
 */
void GetAnimationNameDialog::setup_ui () {

  setInputMode(InputMode::TextInput);
  setLabelText(tr("Animation name:"));
}
