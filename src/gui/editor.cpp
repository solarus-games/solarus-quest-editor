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
#include <QMessageBox>
#include <QUndoStack>
#include <QVBoxLayout>

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
 * @fn Editor::cut
 * @brief Copies the selection to the clipboard and removes it.
 */

/**
 * @fn Editor::copy
 * @brief Copies the selection to the clipboard.
 */

/**
 * @fn Editor::paste
 * @brief Inserts the content of the clipboard.
 */
