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
#include "gui/editor.h"
#include "editor_exception.h"
#include "quest.h"
#include <solarus/SolarusFatal.h>
#include <QMessageBox>
#include <QUndoStack>
#include <QVBoxLayout>
#include <iostream>

namespace {

/**
 * @brief Undo command that wraps another one and does nothing the first time.
 *
 * This allows to use a command that was already executed once before being
 * pushed on a QUndoStack.
 *
 * Once a command is in a QUndoStack, redo() and undo() should never throw
 * exceptions, because it would let the history in an undefined state
 * (with a command partially done).
 * However, if it happens anyway, this class catches the exception and prints
 * an error message.
 */
class UndoCommandSkipFirst : public QUndoCommand {

public:

  /**
   * @brief Constructor.
   * @param undo_stack The undo stack.
   * @param wrapped_command The undo command to wrap.
   */
  UndoCommandSkipFirst(std::unique_ptr<QUndoCommand> wrapped_command):
    QUndoCommand(wrapped_command->text()),
    wrapped_command(std::move(wrapped_command)),
    first_time(true) {

  }

  /**
   * @brief Calls undo() on the wrapped command.
   *
   * The wrapped undo() should not throw exceptions: if it does, this function
   * catches it because it is better than a crash, but the undo stack is then
   * unrecoverable.
   */
  virtual void undo() override {

    try {
      wrapped_command->undo();
    }
    catch (const std::exception& ex) {
      // This is a bug in the editor.
      std::cerr << "Error in undo(): " << ex.what() << std::endl;
    }
  }

  /**
   * @brief Calls redo() on the wrapped command.
   *
   * Does nothing the first time, because the command was already done
   * from the constructor.
   *
   * The wrapped redo() should not throw exceptions if it did not the first
   * time. If it does anyway, this function catches it because it is better
   * than a crash, but the undo stack is then unrecoverable.
   */
  virtual void redo() override {

    if (first_time) {
      // Not a real redo.
      first_time = false;
      return;
    }

    try {
      wrapped_command->redo();
    }
    catch (const std::exception& ex) {
      // This is a bug in the editor.
      std::cerr << "Error in redo(): " << ex.what() << std::endl;
    }
  }

private:

  std::unique_ptr<QUndoCommand>
      wrapped_command;     /**< The text editor widget to
                            * forward undo/redo commands to. */
  bool first_time;         /**< \c true if redo has not been called yet. */
};

}

/**
 * @brief Creates an editor.
 * @param quest The quest the file to edit belongs to.
 * @param file_path Path of the file to edit.
 * @param parent The parent object or nullptr.
 */
Editor::Editor(Quest& quest, const QString& file_path, QWidget* parent) :
  QWidget(parent),
  quest(quest),
  file_path(file_path),
  title(get_file_name()),
  undo_stack(new QUndoStack(this)) {

  // Default close confirmation message.
  set_close_confirm_message(
        tr("File '%1' has been modified. Save changes?").arg(get_file_name()));
}

/**
 * @brief Returns the quest the edited file belongs to.
 * @return The quest.
 */
const Quest& Editor::get_quest() const {
  return quest;
}

/**
 * @brief Returns the quest the edited file belongs to.
 *
 * Non-const version.
 *
 * @return The quest.
 */
Quest& Editor::get_quest() {
  return quest;
}

/**
 * @brief Returns the list of resources of the quest.
 * @return The quest resources.
 */
const QuestResources& Editor::get_resources() const {
  return quest.get_resources();
}

/**
 * @brief Returns the list of resources of the quest.
 *
 * Non-const version.
 *
 * @return The quest resources.
 */
QuestResources& Editor::get_resources() {
  return quest.get_resources();
}

/**
 * @brief Returns the path of the file being edited.
 * @return The file path.
 */
QString Editor::get_file_path() const {
  return file_path;
}

/**
 * @brief Returns the name of the file being edited.
 *
 * The file name is the last component of the path.
 *
 * @return The file name.
 */
QString Editor::get_file_name() const {
  return file_path.section('/', -1, -1, QString::SectionSkipEmpty);
}

/**
 * @brief Returns the name of the file being edited, without extension.
 *
 * The file name is the last component of the path.
 *
 * @return The file name without extension.
 */
QString Editor::get_file_name_without_extension() const {
  return get_file_name().section('.', 0, -2);
}

/**
 * @fn Editor::get_title
 * @brief Returns a user-friendly title for this editor.
 *
 * It is used in the tab label.
 * An asterisk is automatically added in the tab title if the file is modified.
 *
 * The default title is the file name (without its directories).
 *
 * @return A title describing the file edited.
 */
QString Editor::get_title() const {
  return title;
}

/**
 * @brief Sets the title of this editor.
 * @param title The title to set.
 */
void Editor::set_title(const QString& title) {
  this->title = title;
}

/**
 * @brief Returns an icon representing this editor.
 *
 * It is used in the tab bar.
 *
 * @return An icon representing the file edited.
 */
QIcon Editor::get_icon() const {
  return icon;
}

/**
 * @brief Sets the icon representing this editor.
 *
 * It is used in the tab bar.
 *
 * @param icon An icon representing the file edited.
 */
void Editor::set_icon(const QIcon& icon) {
  this->icon = icon;
}

/**
 * @brief Returns the message proposing to save changes when closing.
 * @return The closing confirmation message.
 */
QString Editor::get_close_confirm_message() const {
  return close_confirm_message;
}

/**
 * @brief Returns the message proposing to save changes when closing.
 * @param message The closing confirmation message.
 */
void Editor::set_close_confirm_message(const QString& message) {
  this->close_confirm_message = message;
}

/**
 * @brief Returns the undo/redo history of editing this file.
 * @return The undo/redo history.
 */
QUndoStack& Editor::get_undo_stack() {
  return *undo_stack;
}

/**
 * @brief Attempts to execute a command.
 *
 * Only adds it to the undo/redo history if everything is okay.
 * Otherwise, shows an error dialog if an exception occurs.
 *
 * @param command The command to do.
 * This function takes ownership of the pointer.
 * @return @c true in case of success, @c false if an exception occurred.
 */
bool Editor::try_command(QUndoCommand* command) {

  std::unique_ptr<QUndoCommand> command_ptr(command);
  try {
    command_ptr->redo();  // Exceptions are allowed here.
    // Now we know that the command succeeds.
    // Unfortunately, we cannot directly add it to the undo stack because
    // the undo stack would execute it again.
    // So let's wrap it in a special command.
    get_undo_stack().push(new UndoCommandSkipFirst(std::move(command_ptr)));
    return true;
  }
  catch (const EditorException& ex) {
    // Error from the user.
    ex.show_dialog();
  }
  catch (const Solarus::SolarusFatal& ex) {
    // Internal error of the Solarus library.
    std::cerr << ex.what() << std::endl;
  }
  catch (const std::exception& ex) {
    // Other error.
    std::cerr << ex.what() << std::endl;
  }

  return false;
}

/**
 * @fn Editor::save
 * @brief Saves the file.
 *
 * You don't have to call QUndoStack::setClean() from your save() function:
 * this is automatically done if the save operation is successful.
 *
 * @throws EditorException In case of failure.
 */

/**
 * @brief Function called when the user wants to close the editor.
 *
 * If the file is not saved, a dialog proposes to save it.
 *
 * @return @c false to cancel the closing operation.
 */
bool Editor::confirm_close() {

  if (get_undo_stack().isClean()) {
    // The file is saved.
    return true;
  }

  QMessageBox::StandardButton answer = QMessageBox::question(
        nullptr,
        tr("Save changes"),
        get_close_confirm_message(),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save
        );

  switch (answer) {

  case QMessageBox::Save:
    // Save and close.
    try {
      save();
      get_undo_stack().setClean();
      return true;
    }
    catch (const EditorException& ex) {
      ex.show_dialog();
      return false;
    }

  case QMessageBox::Discard:
    // Close without saving.
    return true;

  case QMessageBox::Cancel:
  case QMessageBox::Escape:
    // Don't close.
    return false;

  default:
    return false;
  }

}

/**
 * @brief Copies the selection to the clipboard and removes it.
 *
 * The default implementation does nothing, meaning that clipboard operations
 * are not supported.
 */
void Editor::cut() {
}

/**
 * @brief Copies the selection to the clipboard.
 *
 * The default implementation does nothing, meaning that clipboard operations
 * are not supported.
 */
void Editor::copy() {
}

/**
 * @brief Inserts the content of the clipboard.
 *
 * The default implementation does nothing, meaning that clipboard operations
 * are not supported.
 */
void Editor::paste() {
}
