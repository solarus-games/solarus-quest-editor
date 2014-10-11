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
#include "gui/gui_tools.h"
#include "gui/tileset_editor.h"
#include "editor_exception.h"
#include "quest.h"
#include "quest_resources.h"

/**
 * @brief Creates a tileset editor.
 * @param quest The quest containing the file.
 * @param path Path of the tileset data file to open.
 * @param parent The parent object or nullptr.
 * @throws EditorException If the file could not be opened.
 */
TilesetEditor::TilesetEditor(Quest& quest, const QString& path, QWidget* parent) :
  Editor(quest, path, parent) {

  // Get the tileset.
  ResourceType resource_type;
  QString tileset_id;
  quest.check_exists(path);
  if (!quest.is_resource_element(path, resource_type, tileset_id) ||
      resource_type != ResourceType::TILESET) {
    throw EditorException("File '%1' is not a tileset");
  }
  this->tileset_id = tileset_id;

  // Prepare the gui.
  ui.setupUi(this);
  layout()->addWidget(ui.splitter);
  const int side_width = 300;
  ui.splitter->setSizes(QList<int>() << side_width << width() - side_width);

  ui.tileset_id_field->setText(tileset_id);
  update_description_to_gui();

  // Make connections.
  connect(&get_resources(), SIGNAL(element_description_changed(ResourceType, const QString&, const QString&)),
          this, SLOT(update_description_to_gui()));
  connect(ui.description_field, SIGNAL(editingFinished()),
          this, SLOT(set_description_from_gui()));
}

/**
 * @copydoc Editor::get_title
 */
QString TilesetEditor::get_title() const {

  return "Tileset " + get_file_name_without_extension();
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

/**
 * @brief Updates the content of the tileset description text edit.
 */
void TilesetEditor::update_description_to_gui() {

  QString description = get_resources().get_description(ResourceType::TILESET, tileset_id);
  if (ui.description_field->text() != description) {
    ui.description_field->setText(description);
  }
}

/**
 * @brief Modifies the tileset description in the quest resource list with
 * the new text entered by the user.
 *
 * If the new description is invalid, an error dialog is shown.
 */
void TilesetEditor::set_description_from_gui() {

  QString description = ui.description_field->text();
  if (description == get_resources().get_description(ResourceType::TILESET, tileset_id)) {
    return;
  }

  if (description.isEmpty()) {
    GuiTools::errorDialog("Invalid description");
    update_description_to_gui();
    return;
  }

  blockSignals(true);  // TODO scoped blocker
  get_resources().set_description(ResourceType::TILESET, tileset_id, description);
  get_resources().save();
  blockSignals(false);
}
