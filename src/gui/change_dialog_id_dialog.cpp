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
#include "gui/change_dialog_id_dialog.h"
#include "gui/gui_tools.h"
#include "dialogs_model.h"

/**
 * @brief Creates a change dialog id dialog.
 * @param initial_id Initial key of the string.
 * @param is_prefix If the id is a prefix.
 * @param allow_prefix Allows to change the prefix value.
 * @param parent Parent object or nullptr.
 */
ChangeDialogIdDialog::ChangeDialogIdDialog (
    DialogsModel *model, const QString &initial_id,
    bool is_prefix, bool allow_prefix, QWidget *parent) :
  QDialog(parent),
  initial_id(initial_id),
  model(model) {

  ui.setupUi(this);

  if (!allow_prefix && is_prefix) {
    ui.dialog_id_label->setText(
      tr("New id for dialogs prefixed by '%1':").arg(initial_id));
  } else {
    ui.dialog_id_label->setText(
      tr("New id for dialog '%1':").arg(initial_id));
  }
  ui.prefix_mode_field->setEnabled(allow_prefix);

  set_dialog_id(initial_id);
  set_prefix(is_prefix);

  ui.dialog_id_field->selectAll();
}

/**
 * @brief Returns dialog id entered by the user.
 * @return The dialog id.
 */
QString ChangeDialogIdDialog::get_dialog_id() const {

  return ui.dialog_id_field->text();
}

/**
 * @brief Sets the dialog id displayed in the text edit.
 * @param id The id to set.
 */
void ChangeDialogIdDialog::set_dialog_id(const QString& id) {

  ui.dialog_id_field->setText(id);
}

/**
 * @brief Returns prefix value entered by the user.
 * @return The prefix value.
 */
bool ChangeDialogIdDialog::get_prefix() const {

  return ui.prefix_mode_field->isChecked();
}

/**
 * @brief Sets the prefix value displayed in the text edit.
 * @param prefix The prefix to set.
 */
void ChangeDialogIdDialog::set_prefix(bool prefix) {

  ui.prefix_mode_field->setChecked(prefix);
}

/**
 * @brief Closes the dialog unless the user tries to set invalid data.
 * @param result Result code of the dialog.
 */
void ChangeDialogIdDialog::done(int result) {

  if (result == QDialog::Accepted) {

    QString id = get_dialog_id();

    if (!DialogsModel::is_valid_id(id)) {
      GuiTools::error_dialog(tr("Invalid dialog id: %1").arg(id));
      return;
    }

    if (id != initial_id) {
      if (get_prefix()) {
        QString error_id;
        if (!model->can_set_dialog_id_prefix(initial_id, id, error_id)) {
          GuiTools::error_dialog(
                tr("The dialog '%1' already exists").arg(error_id));
          return;
        }
      } else if (model->dialog_exists(id)) {
        GuiTools::error_dialog(
              tr("The dialog '%1' already exists").arg(id));
        return;
      }
    }
  }

  QDialog::done(result);
}
