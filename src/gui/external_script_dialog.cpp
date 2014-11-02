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
#include "gui/external_script_dialog.h"
#include <QPushButton>

/**
 * Creates a script dialog.
 * @param title Title of the dialog box.
 * @param script_path The script to run, without extension.
 * @param arg An argument to pass to the script, or an empty string.
 * @param parent The parent object or nullptr.
 */
ExternalScriptDialog::ExternalScriptDialog(
    const QString& title,
    const QString& script_path,
    const QString& arg,
    QWidget* parent) :
  QDialog(parent) {

  // TODO
  Q_UNUSED(arg);
  Q_UNUSED(title);
  Q_UNUSED(script_path);
  Q_UNUSED(parent);

  ui.setupUi(this);

  // Disable the Ok button until the script is finished.
  QPushButton* button = ui.button_box->button(QDialogButtonBox::Ok);
  if (button != nullptr) {
    button->setEnabled(false);
  }
  setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
}

/**
 * @brief Returns whether the script is finished.
 * @return @c true if the run is finished.
 */
bool ExternalScriptDialog::is_finished() const {
  // TODO
  return false;
}

/**
 * @brief Returns whether the script ran successfully.
 * @return @c true if the run was successful.
 */
bool ExternalScriptDialog::is_successful() const {
  // TODO
  return true;
}

/**
 * @brief Receives a close event.
 * @param event The event to handle.
 */
void ExternalScriptDialog::closeEvent(QCloseEvent* event) {

  if (!is_finished()) {
    // Don't allow to close the dialog while the script is running.
    event->ignore();
  }
  else {
    QDialog::closeEvent(event);
  }
}
