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
#include "gui/tileset_editor.h"

/**
 * @brief Creates a tileset editor.
 * @param quest The quest containing the file.
 * @param path Path of the tileset data file to open.
 * @param parent The parent object or nullptr.
 * @throws QuestEditorException If the file could not be opened.
 */
TilesetEditor::TilesetEditor(Quest& quest, const QString& path, QWidget* parent) :
  Editor(quest, path, parent) {

  ui.setupUi(this);
  layout()->addWidget(ui.splitter);
}

/**
 * @copydoc Editor::get_title
 */
QString TilesetEditor::get_title() const {

  // TODO
  return "Tileset";
}

/**
 * @copydoc Editor::get_icon
 */
QIcon TilesetEditor::get_icon() const {

  return QIcon(":/images/icon_resource_tileset.png");
}

/**
 * @copydoc Editor::is_modified
 */
bool TilesetEditor::is_modified() const {

  // TOOD
  return false;
}

/**
 * @copydoc Editor::save
 */
void TilesetEditor::save() {

  // TOOD
}

/**
 * @copydoc Editor::confirm_close
 */
bool TilesetEditor::confirm_close() {

  // TOOD
  return true;
}

/**
 * @copydoc Editor::undo
 */
void TilesetEditor::undo() {

  // TOOD
}

/**
 * @copydoc Editor::redo
 */
void TilesetEditor::redo() {

  // TOOD
}

/**
 * @copydoc Editor::cut
 */
void TilesetEditor::cut() {

  // TOOD
}

/**
 * @copydoc Editor::copy
 */
void TilesetEditor::copy() {

  // TOOD
}

/**
 * @copydoc Editor::paste
 */
void TilesetEditor::paste() {

  // TOOD
}
