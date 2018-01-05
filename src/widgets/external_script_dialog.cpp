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
#include "widgets/external_script_dialog.h"
#include <lua.hpp>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QPushButton>
#include <QTemporaryFile>
#include <QTimer>

namespace SolarusEditor {

namespace {

/**
 * @brief Custom loader for Lua to require() files from the script's directory.
 *
 * It also works if the script is inside Qt resources.
 *
 * This function expects an upvalue of type string indicating the path of the
 * current script (necessary to know its directory).
 *
 * @param l A Lua state.
 * @return Number of values to return to Lua.
 */
int l_loader_from_current_dir(lua_State* l) {

  QString current_script_path = QString::fromUtf8(lua_tostring(l, lua_upvalueindex(1)));
  QString required_name = QString::fromUtf8(luaL_checkstring(l, 1));

  if (QFileInfo(required_name).isAbsolute()) {
    // An absolute path was specified: not our job here (and it is not an error).
    lua_pushnil(l);
    return 1;
  }

  QDir parent_dir(current_script_path);
  if (!parent_dir.cdUp()) {
    // Something must be wrong in this C++ code.
    qCritical() << ExternalScriptDialog::tr("Cannot determine the directory of script '%1'").arg(current_script_path);
    lua_pushnil(l);
    return 1;
  }

  QString required_path = parent_dir.absolutePath() + "/" + required_name + ".lua";

  QFile required_file(required_path);
  if (!required_file.open(QFileDevice::ReadOnly)) {
    // File was not found by this loader.
    // This is not an error, but we should give information about what was
    // searched, like standard loaders.

    // Note that the information is not translated because Lua uses it
    // to build a larger diagnostic in English.
    QByteArray message;
    if (required_path.startsWith(":/")) {
      // Qt resource that was searched in the script's directory.
      message = QString("\n\tno Qt resource file '%1'").arg(required_path).toUtf8();
    }
    else {
      // Regular file that was searched in the script's directory.
      message = QString("\n\tno file '%1'").arg(required_path).toUtf8();
    }
    lua_pushstring(l, message.constData());
    return 1;
  }

  QByteArray buffer = required_file.readAll();
  QByteArray required_path_utf8 = required_path.toUtf8();
  luaL_loadbuffer(l, buffer.constData(), buffer.size(), required_path_utf8);
  return 1;
}

}

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
  ui.description_label->setText(title + "... ");
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

  ui.status_label->setText(tr("In progress"));
  ui.status_label->setStyleSheet("font-weight: bold; color: orange");

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
 *
 * This function does not throw exceptions, it outputs any error to the
 * text widget.
 */
void ExternalScriptDialog::run_script() {

  successful = false;
  QString output;

  lua_State* l = luaL_newstate();
  luaL_openlibs(l);

  QString path = script_path + ".lua";
  QFile script_file(path);
  if (!script_file.open(QFileDevice::ReadOnly)) {
    output = tr("Cannot open file '%1'").arg(path);
  }
  else {
    QByteArray buffer = script_file.readAll();
    QByteArray path_utf8 = path.toUtf8();
    if (luaL_loadbuffer(l, buffer.constData(), buffer.size(), path_utf8.constData()) != 0) {
      // Loading the script failed.
      output = QString::fromStdString(std::string(lua_tostring(l, -1)));
    }
    else {
      // Make require able to find files relative to the script's directory.
      lua_pushstring(l, path_utf8.constData());
      lua_pushcclosure(l, l_loader_from_current_dir, 1);
      lua_setglobal(l, "loader_from_current_dir");
      luaL_dostring(l, "table.insert(package.loaders, 2, loader_from_current_dir)");  // TODO clean this

      int num_arguments = 0;
      if (!script_arg.isEmpty()) {
        num_arguments = 1;
        lua_pushstring(l, script_arg.toUtf8().constData());
      }

      // Redirect io.write() calls to a temporary file.
      QTemporaryFile log_file;
      log_file.open();   // Necessary to generate a name for the temporary file.
      log_file.close();  // But close it: is it actually Lua that will write it
                         // (we just wanted its name).
      QByteArray lua_instruction_utf8 = ("io.output(\"" + log_file.fileName() + "\")").toUtf8();
      luaL_dostring(l, lua_instruction_utf8.constData());

      // Run the script.
      if (lua_pcall(l, num_arguments, 0, 0) == 0) {
        successful = true;
      }

      log_file.open();
      output = QString::fromUtf8(log_file.readAll());
    }
  }

  lua_close(l);
  ui.output_field->appendPlainText(output);
  if (successful) {
    ui.status_label->setText(tr("Successful!"));
    ui.status_label->setStyleSheet("font-weight: bold; color: green");
  }
  else {
    ui.status_label->setText(tr("Failure"));
    ui.status_label->setStyleSheet("font-weight: bold; color: red");
  }

  set_finished(true);
}

}
