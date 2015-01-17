/*
 * Copyright (C) 2014-2015 Christopho, Solarus - http://www.solarus-games.org
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
#include "gui/map_editor.h"
#include "editor_exception.h"
#include "map_model.h"
#include "quest.h"
#include "quest_resources.h"
#include <QUndoStack>

/**
 * @brief Creates a map editor.
 * @param quest The quest containing the file.
 * @param path Path of the map data file to open.
 * @param parent The parent object or nullptr.
 * @throws EditorException If the file could not be opened.
 */
MapEditor::MapEditor(Quest& quest, const QString& path, QWidget* parent) :
  Editor(quest, path, parent),
  model(nullptr) {

  ui.setupUi(this);

  // Get the map.
  ResourceType resource_type;
  QString map_id;
  quest.check_exists(path);
  if (!quest.is_resource_element(path, resource_type, map_id) ||
      resource_type != ResourceType::MAP) {
    throw EditorException("File '%1' is not a map");
  }
  this->map_id = map_id;

  // Editor properties.
  set_title("Map " + get_file_name_without_extension());
  set_icon(QIcon(":/images/icon_resource_map.png"));
  set_close_confirm_message(
        tr("Map '%1' has been modified. Save changes?").arg(map_id));
  set_zoom_supported(true);
  set_zoom(2.0);

  // Open the file.
  model = new MapModel(quest, map_id, this);
  get_undo_stack().setClean();

  // Prepare the gui.
  const int side_width = 350;
  ui.splitter->setSizes({ side_width, width() - side_width });
  ui.tileset_field->set_resource_type(ResourceType::TILESET);
  ui.tileset_field->set_quest(quest);
  ui.music_field->set_resource_type(ResourceType::MUSIC);
  ui.music_field->set_quest(quest);
  ui.tileset_view->set_read_only(true);
  update();

  // Make connections.
  connect(&get_resources(), SIGNAL(element_description_changed(ResourceType, const QString&, const QString&)),
          this, SLOT(update_description_to_gui()));
  connect(ui.description_field, SIGNAL(editingFinished()),
          this, SLOT(set_description_from_gui()));
}

/**
 * @copydoc Editor::save
 */
void MapEditor::save() {

  model->save();
}

/**
 * @brief Updates everything in the gui.
 */
void MapEditor::update() {

  update_map_id_field();
  update_description_to_gui();
  update_tileset_view();
}

/**
 * @brief Updates the map id displaying.
 */
void MapEditor::update_map_id_field() {

  ui.map_id_field->setText(map_id);
}

/**
 * @brief Updates the content of the map description text edit.
 */
void MapEditor::update_description_to_gui() {

  QString description = get_resources().get_description(ResourceType::MAP, map_id);
  if (ui.description_field->text() != description) {
    ui.description_field->setText(description);
  }
}

/**
 * @brief Modifies the map description in the quest resource list with
 * the new text entered by the user.
 *
 * If the new description is invalid, an error dialog is shown.
 */
void MapEditor::set_description_from_gui() {

  QString description = ui.description_field->text();
  if (description == get_resources().get_description(ResourceType::MAP, map_id)) {
    return;
  }

  if (description.isEmpty()) {
    GuiTools::error_dialog("Invalid description");
    update_description_to_gui();
    return;
  }

  QSignalBlocker(this);
  try {
    get_resources().set_description(ResourceType::MAP, map_id, description);
    get_resources().save();
  }
  catch (const EditorException& ex) {
    ex.print_message();
  }
}

/**
 * @brief Updates the content of the tileset view.
 */
void MapEditor::update_tileset_view() {

  ui.tileset_view->set_model(model->get_tileset_model());
}
