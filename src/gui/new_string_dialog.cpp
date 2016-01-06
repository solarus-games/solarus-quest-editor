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
#include "gui/new_string_dialog.h"
#include "gui/gui_tools.h"
#include "strings_model.h"

/**
 * @brief Creates a change string key dialog.
 * @param key Key of the string to change.
 * @param initial_value Initial value of the string.
 * @param parent Parent object or nullptr.
 */
NewStringDialog::NewStringDialog (
    StringsModel *model, const QString &initial_key,
    const QString &initial_value, QWidget *parent) :
  QDialog(parent),
  model(model) {

  ui.setupUi(this);
  set_string_key(initial_key);
  set_string_value(initial_value);
}

/**
 * @brief Returns string key entered by the user.
 * @return The string key.
 */
QString NewStringDialog::get_string_key() const {

  return ui.string_key_field->text();
}

/**
 * @brief Sets the string key displayed in the text edit.
 * @param key The key to set.
 */
void NewStringDialog::set_string_key(const QString& key) {

  ui.string_key_field->setText(key);
}

/**
 * @brief Returns string value entered by the user.
 * @return The string value.
 */
QString NewStringDialog::get_string_value() const {

  return ui.string_value_field->text();
}

/**
 * @brief Sets the string value displayed in the text edit.
 * @param value The value to set.
 */
void NewStringDialog::set_string_value(const QString& value) {

  ui.string_value_field->setText(value);
}

/**
 * @brief Closes the dialog unless the user tries to set invalid data.
 * @param result Result code of the dialog.
 */
void NewStringDialog::done(int result) {

  if (result == QDialog::Accepted) {

    QString key = get_string_key();
    QString value = get_string_value();

    if (!StringsModel::is_valid_key(key)) {
      GuiTools::error_dialog("Invalid string key");
      return;
    }

    if (model->string_exists(key)) {
      GuiTools::error_dialog("This string key already exists");
      return;
    }

    if (value.isEmpty()) {
      GuiTools::error_dialog("Value cannot be empty");
      return;
    }
  }

  QDialog::done(result);
}
