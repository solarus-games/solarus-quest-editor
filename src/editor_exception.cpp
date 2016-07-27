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
#include "widgets/gui_tools.h"
#include "editor_exception.h"
#include <iostream>

namespace SolarusEditor {

/**
 * @brief Creates a quest editor exception.
 * @param message The error message.
 */
EditorException::EditorException(const QString& message) :
  message(message),
  message_utf8() {

}

/**
 * @brief Returns the error message.
 * @return The error message.
 */
QString EditorException::get_message() const noexcept {
  return message;
}

/**
 * @brief Returns the error message.
 * @return The error message.
 * It will no longer be valid after the exception is destroyed.
 */
const char* EditorException::what() const noexcept {

  message_utf8 = message.toUtf8();
  return message_utf8.constData();
}

/**
 * @brief Prints the error message on stderr->
 */
void EditorException::print_message() const {

  std::cerr << message.toStdString() << std::endl;
}

/**
 * @brief Shows a modal dialog box with the error message.
 */
void EditorException::show_dialog() const {

  GuiTools::error_dialog(message);
}

}
