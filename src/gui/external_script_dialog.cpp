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
#include <QTimer>

/**
 * Creates a script dialog.
 * @param title Title describing the operation.
 * @param script_path The script to run, without extension.
 * @param script_arg An argument to pass to the script, or an empty string.
 * @param parent The parent object or nullptr.
 */
ExternalScriptDialog::ExternalScriptDialog(
    const QString& title,
    const QString& script_path,
    const QString& script_arg,
    QWidget* parent) :
  QDialog(parent),
  script_path(script_path),
  script_arg(script_arg),
  finished(false),
  successful(false) {

  ui.setupUi(this);

  set_finished(false);
  setWindowTitle(title);
  ui.status_label->setText(title + "...");
}

/**
 * @brief Returns whether the script is finished.
 * @return @c true if the run is finished.
 */
bool ExternalScriptDialog::is_finished() const {
  return finished;
}

/**
 * @brief Sets whether the script is finished.
 * @param finished @c true if the run is finished.
 */
void ExternalScriptDialog::set_finished(bool finished) {

  this->finished = finished;

  // Disable the Ok button until the script is finished.
  QPushButton* button = ui.button_box->button(QDialogButtonBox::Ok);
  if (button != nullptr) {
    button->setEnabled(finished);
  }
}

/**
 * @brief Returns whether the script is successfully finished.
 * @return @c true if the script is finished and was successful.
 */
bool ExternalScriptDialog::is_successful() const {
  return successful;
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

/**
 * @brief Executes the dialog, blocking until the script runs and the dialog
 * is closed.
 */
int ExternalScriptDialog::exec() {

  // Use a timer to show the window before running the script.
  // The script will start at the next event loop iteration.
  // A thread could be used to show the output in real time,
  // but the timer solution is enough for us.
  QTimer* timer = new QTimer(this);
  timer->setSingleShot(true);
  connect(timer, SIGNAL(timeout()), this, SLOT(run_script()));
  timer->start();

  return QDialog::exec();
}

/**
 * @brief Runs the script. Returns when the script is finished.
 */
void ExternalScriptDialog::run_script() {

  // TODO
  successful = true;
  set_finished(true);
}
