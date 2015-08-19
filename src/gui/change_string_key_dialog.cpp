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
#include "gui/change_string_key_dialog.h"
#include "gui/gui_tools.h"
#include "strings_model.h"

/**
 * @brief Creates a change string key dialog.
 * @param initial_key Initial key of the string.
 * @param is_prefix If the key is a prefix key.
 * @param allow_prefix Allows to change the prefix value.
 * @param parent Parent object or nullptr.
 */
ChangeStringKeyDialog::ChangeStringKeyDialog (
    StringsModel *model, const QString &initial_key,
    bool is_prefix, bool allow_prefix, QWidget *parent) :
  QDialog(parent),
  initial_key(initial_key),
  model(model) {

  ui.setupUi(this);

  if (!allow_prefix && is_prefix) {
    ui.string_key_label->setText(
      tr("New key for strings prefixed by '%1':").arg(initial_key));
  } else {
    ui.string_key_label->setText(
      tr("New key for string '%1':").arg(initial_key));
  }
  ui.prefix_mode_field->setEnabled(allow_prefix);

  set_string_key(initial_key);
  set_prefix(is_prefix);

  ui.string_key_field->selectAll();
}

/**
 * @brief Returns string key entered by the user.
 * @return The string key.
 */
QString ChangeStringKeyDialog::get_string_key() const {

  return ui.string_key_field->text();
}

/**
 * @brief Sets the string key displayed in the text edit.
 * @param key The key to set.
 */
void ChangeStringKeyDialog::set_string_key(const QString& key) {

  ui.string_key_field->setText(key);
}

/**
 * @brief Returns prefix value entered by the user.
 * @return The prefix value.
 */
bool ChangeStringKeyDialog::get_prefix() const {

  return ui.prefix_mode_field->isChecked();
}

/**
 * @brief Sets the prefix value displayed in the text edit.
 * @param prefix The prefix to set.
 */
void ChangeStringKeyDialog::set_prefix(bool prefix) {

  ui.prefix_mode_field->setChecked(prefix);
}

/**
 * @brief Closes the dialog unless the user tries to set invalid data.
 * @param result Result code of the dialog.
 */
void ChangeStringKeyDialog::done(int result) {

  if (result == QDialog::Accepted) {

    QString key = get_string_key();

    if (!StringsModel::is_valid_key(key)) {
      GuiTools::error_dialog(tr("Invalid string key: %1").arg(key));
      return;
    }

    if (key != initial_key) {
      if (get_prefix()) {
        QString error_key;
        if (!model->can_set_string_key_prefix(initial_key, key, error_key)) {
          GuiTools::error_dialog(
                tr("The string '%1' already exists").arg(error_key));
          return;
        }
      } else if (model->string_exists(key)) {
        GuiTools::error_dialog(
              tr("The string '%1' already exists").arg(key));
        return;
      }
    }
  }

  QDialog::done(result);
}
