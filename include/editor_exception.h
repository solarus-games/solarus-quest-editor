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
#ifndef SOLARUSEDITOR_EDITOR_EXCEPTION_H
#define SOLARUSEDITOR_EDITOR_EXCEPTION_H

#include <QString>
#include <exception>

namespace SolarusEditor {

/**
 * @brief Exception thrown if something wrong happens in the quest editor.
 */
class EditorException : public std::exception {

public:

  EditorException(const QString& message);
  virtual ~EditorException() = default;

  QString get_message() const noexcept;
  const char* what() const noexcept override;

  void print_message() const;
  void show_dialog() const;

private:

  QString message;                     /**< The error message. */
  mutable QByteArray message_utf8;     /**< The error message in UTF-8. */

};

}

#endif
