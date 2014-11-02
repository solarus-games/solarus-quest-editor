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
#include <QFileInfo>
#include <QPushButton>
#include <QTimer>
#include <lua.hpp>
#include <sstream>
#include <string>

namespace {

/**
 * @brief Custom loader for Lua to lookup files in Qt resources using require().
 * @param l A Lua state.
 * @return Number of values to return to Lua.
 */
int l_loader_from_qt_resource(lua_State* l) {

  const std::string& script_name = luaL_checkstring(l, 1);
  std::string path(std::string(":/") + script_name);
  bool exists = false;

  QFile script_file(QString::fromStdString(path));
  if (script_file.open(QFileDevice::ReadOnly)) {
    QByteArray buffer = script_file.readAll();
    luaL_loadbuffer(l, buffer.constData(), buffer.size(), path.c_str());
  }

  if (!exists) {
    std::ostringstream oss;
    oss << std::endl << "\tno file ':/" << script_name
        << ".lua' in Qt resources";
    lua_pushstring(l, oss.str().c_str());
  }

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
    QByteArray chunk_name = path.toUtf8();
    if (luaL_loadbuffer(l, buffer.constData(), buffer.size(), chunk_name.constData()) != 0) {
      output = QString::fromStdString(std::string(lua_tostring(l, -1)));
    }
    else {

      if (path.startsWith(":/")) {
        // Make require able to find files inside Qt resources.
        lua_pushcfunction(l, l_loader_from_qt_resource);
        lua_setglobal(l, "loader_from_qt_resource");
        luaL_dostring(l, "table.insert(package.loaders, 2, loader_from_qt_resource)");
      }

      int num_arguments = 0;
      if (!script_arg.isEmpty()) {
        num_arguments = 1;
        lua_pushstring(l, script_arg.toUtf8().constData());
      }
      if (lua_pcall(l, num_arguments, 0, 0) != 0) {
        output = QString::fromStdString(std::string(lua_tostring(l, -1)));
      }
      else {
        // TODO
        output = "success";
        successful = true;
      }
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
