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
#include "quest.h"
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
  file_path(file_path) {

}

/**
 * @brief Returns the quest the edited file belongs to.
 * @return The quest.
 */
const Quest& Editor::get_quest() const {
  return quest;
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
  return get_file_name();
}

/**
 * @fn Editor::get_icon
 * @brief Returns an icon representing this editor.
 *
 * It is used in the tab bar.
 *
 * @return An icon representing the file edited.
 */

/**
 * @fn Editor::is_modified
 * @brief Returns whether the file being edited has been modified.
 * @return @c true if the file is modified, @c false if it is saved.
 */

/**
 * @fn Editor::save
 * @brief Saves the file.
 */

/**
 * @fn Editor::confirm_close
 * @brief Function called when the user wants to close the editor.
 *
 * If the file is not saved, you should propose to save it.
 *
 * @return @c false to cancel the closing operation.
 */

/**
 * @fn Editor::undo
 * @brief Undoes the last action.
 */

/**
 * @fn Editor::redo
 * @brief Redoes the previous undone action.
 */

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
