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
#include "gui/console.h"
#include "quest_runner.h"

/**
 * @brief Creates a console view.
 * @param parent Parent object or nullptr.
 */
Console::Console(QWidget* parent) :
  QWidget(parent) {

  ui.setupUi(this);

}

/**
 * @brief Connects this console to a quest runner.
 * @param quest_runner The quest runner.
 */
void Console::set_quest_runner(QuestRunner& quest_runner) {

  this->quest_runner = &quest_runner;

  connect(ui.command_field, SIGNAL(returnPressed()),
          this, SLOT(command_field_activated()));

  connect(&quest_runner, SIGNAL(running()),
          this, SLOT(quest_running()));
  connect(&quest_runner, SIGNAL(finished()),
          this, SLOT(quest_finished()));
  connect(&quest_runner, SIGNAL(output_produced(QStringList)),
          this, SLOT(quest_output_produced(QStringList)));

}

/**
 * @brief Slot called when the quest execution begins.
 */
void Console::quest_running() {

  ui.command_field->setEnabled(true);
}

/**
 * @brief Slot called when the quest execution is finished.
 */
void Console::quest_finished() {

  ui.command_field->setEnabled(false);
}

/**
 * @brief Slot called when the quest execution produced some output lines.
 * @param lines The lines read from the standard output of the quest.
 */
void Console::quest_output_produced(const QStringList& lines) {

  for (const QString& line : lines) {

    // TODO colorize warnings and errors

    if (line.startsWith("[Solarus]") &&
        (line.contains(" ====== Begin Lua command #") || line.contains(" ====== End Lua command #"))) {
      // Filter special markers indicating the output of a command from the console.
      continue;
    }

    ui.log_view->appendPlainText(line);
  }
}

/**
 * @brief Slot called when the user wants to execute a Lua instruction from the console.
 */
void Console::command_field_activated() {

  if (quest_runner == nullptr) {
    return;
  }
  if (!quest_runner->is_running()) {
    return;
  }

  const QString& command = ui.command_field->text();

  // TODO check Lua syntax before sending it to the quest.

  quest_runner->execute_command(command);
  ui.command_field->clear();

  // Show the command in the log view.
  // TODO: show it only when receiving its results, to make sure it is displayed
  // just before its results.
  ui.log_view->appendPlainText("> " + command);
}
