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
#include "gui/console_line_edit.h"
#include "settings.h"
#include <QKeyEvent>

namespace {

constexpr int max_history_size = 100;

}  // Anonymous namespace

/**
 * @brief Creates a console line field.
 * @param parent Parent object or nullptr.
 */
ConsoleLineEdit::ConsoleLineEdit(QWidget* parent) :
  QLineEdit(parent),
  history(),
  history_position(0),
  current_command() {

  Settings settings;
  history = settings.get_value_string_list(Settings::console_history);
  set_history_position(history.size());  // Start after the history.
}

/**
 * @brief Reimplementation to navigate in the command history with up/down arrows.
 * @param event The event to handle.
 */
void ConsoleLineEdit::keyPressEvent(QKeyEvent* event) {

  switch (event->key()) {

  case Qt::Key_Up:
    history_previous();
    event->accept();
    return;

  case Qt::Key_Down:
    history_next();
    event->accept();
    return;

  }

  QLineEdit::keyPressEvent(event);
}

/**
 * @brief Shows the command at the given history position.
 * @param history_position The new history position to set.
 */
void ConsoleLineEdit::set_history_position(int history_position) {

  Q_ASSERT(history_position >= 0 && history_position <= history.size());

  this->history_position = history_position;

  if (history_position == history.size()) {
    // Typing a new command.
    setText(current_command);
  }
  else {
    // Browsing the history.
    setText(history[history_position]);
  }
}

/**
 * @brief Sets the text from the previous command in the history.
 *
 * Does nothing if there is no such command.
 */
void ConsoleLineEdit::history_previous() {

  if (history_position <= 0) {
    return;
  }

  if (history_position == history.size()) {
    // Remember the new command being typed.
    current_command = text();
  }

  set_history_position(history_position - 1);
}

/**
 * @brief Sets the text from the next command in the history.
 *
 * Does nothing if there is no such command.
 */
void ConsoleLineEdit::history_next() {

  if (history_position >= history.size()) {
    return;
  }

  set_history_position(history_position + 1);
}

/**
 * @brief Adds a command to the history.
 * @param command The command just executed.
 */
void ConsoleLineEdit::command_executed(const QString& command) {

  // Avoid consecutive duplicates.
  if (history.size() == 0 || command != history.last()) {
    history.append(command);

    // Limit the history size.
    while (history.size() > max_history_size) {
      history.removeFirst();
    }

    // Save the modified history.
    Settings settings;
    settings.set_value(Settings::console_history, history);
  }

  current_command.clear();
  set_history_position(history.size());
}
