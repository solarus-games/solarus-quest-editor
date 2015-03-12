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
#include "entities/entity_traits.h"
#include "gui/gui_tools.h"
#include "gui/map_editor.h"
#include "editor_exception.h"
#include "map_model.h"
#include "quest.h"
#include "quest_resources.h"
#include <QStatusBar>
#include <QToolBar>
#include <QUndoStack>

namespace {

constexpr int move_entities_command_id = 1;

/**
 * @brief Parent class of all undoable commands of the map editor.
 */
class MapEditorCommand : public QUndoCommand {

public:

  MapEditorCommand(MapEditor& editor, const QString& text) :
    QUndoCommand(text),
    editor(editor) {
  }

  MapEditor& get_editor() const {
    return editor;
  }

  MapModel& get_model() const {
    return editor.get_model();
  }

private:

  MapEditor& editor;

};

/**
 * @brief Changing the size of the map.
 */
class SetSizeCommand : public MapEditorCommand {

public:
  SetSizeCommand(MapEditor& editor, const QSize& size) :
    MapEditorCommand(editor, MapEditor::tr("Map size")),
    before(get_model().get_size()),
    after(size) { }

  void undo() override { get_model().set_size(before); }
  void redo() override { get_model().set_size(after); }

private:
  QSize before, after;
};

/**
 * @brief Changing the world of the map.
 */
class SetWorldCommand : public MapEditorCommand {

public:
  SetWorldCommand(MapEditor& editor, const QString& world) :
    MapEditorCommand(editor, MapEditor::tr("Map world")),
    before(get_model().get_world()),
    after(world) { }

  void undo() override { get_model().set_world(before); }
  void redo() override { get_model().set_world(after); }

private:
  QString before, after;
};

/**
 * @brief Changing the floor of the map.
 */
class SetFloorCommand : public MapEditorCommand {

public:
  SetFloorCommand(MapEditor& editor, int floor) :
    MapEditorCommand(editor, MapEditor::tr("Map floor")),
    before(get_model().get_floor()),
    after(floor) { }

  void undo() override { get_model().set_floor(before); }
  void redo() override { get_model().set_floor(after); }

private:
  int before, after;
};

/**
 * @brief Changing the location of the map.
 */
class SetLocationCommand : public MapEditorCommand {

public:
  SetLocationCommand(MapEditor& editor, const QPoint& location) :
    MapEditorCommand(editor, MapEditor::tr("Map location")),
    before(get_model().get_location()),
    after(location) { }

  void undo() override { get_model().set_location(before); }
  void redo() override { get_model().set_location(after); }

private:
  QPoint before, after;
};

/**
 * @brief Changing the tileset of the map.
 */
class SetTilesetCommand : public MapEditorCommand {

public:
  SetTilesetCommand(MapEditor& editor, const QString& tileset_id) :
    MapEditorCommand(editor, MapEditor::tr("Tileset")),
    before(get_model().get_tileset_id()),
    after(tileset_id) { }

  void undo() override { get_model().set_tileset_id(before); }
  void redo() override { get_model().set_tileset_id(after); }

private:
  QString before, after;
};

/**
 * @brief Changing the music of the map.
 */
class SetMusicCommand : public MapEditorCommand {

public:
  SetMusicCommand(MapEditor& editor, const QString& music_id) :
    MapEditorCommand(editor, MapEditor::tr("Music")),
    before(get_model().get_music_id()),
    after(music_id) { }

  void undo() override { get_model().set_music_id(before); }
  void redo() override { get_model().set_music_id(after); }

private:
  QString before, after;
};

/**
 * @brief Moving entities on the map.
 */
class MoveEntitiesCommand : public MapEditorCommand {

public:
  MoveEntitiesCommand(MapEditor& editor, const QList<EntityIndex>& indexes, const QPoint& translation) :
    MapEditorCommand(editor, MapEditor::tr("Move entities")),
    indexes(indexes),
    translation(translation) { }

  void undo() override {
    for (const EntityIndex& index : indexes) {
      get_model().add_entity_xy(index, -translation);
    }
  }

  void redo() override {
    for (const EntityIndex& index : indexes) {
      get_model().add_entity_xy(index, translation);
    }
  }

  int id() const override {
    return move_entities_command_id;
  }

  bool mergeWith(const QUndoCommand* other_command) override {

    if (other_command->id() != id()) {
      return false;
    }
    const MoveEntitiesCommand& other_move_command = *static_cast<const MoveEntitiesCommand*>(other_command);
    if (other_move_command.indexes != indexes) {
      return false;
    }

    translation += other_move_command.translation;
    return true;
  }

private:
  QList<EntityIndex> indexes;
  QPoint translation;
};

}  // Anonymous namespace.

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
  build_entity_creation_toolbar();
  build_status_bar();

  // Get the map.
  ResourceType resource_type;
  QString map_id;
  quest.check_exists(path);
  if (!quest.is_resource_element(path, resource_type, map_id) ||
      resource_type != ResourceType::MAP) {
    throw EditorException(tr("File '%1' is not a map").arg(path));
  }
  this->map_id = map_id;

  // Editor properties.
  set_title(tr("Map %1").arg(get_file_name_without_extension()));
  set_icon(QIcon(":/images/icon_resource_map.png"));
  set_close_confirm_message(
        tr("Map '%1' has been modified. Save changes?").arg(map_id));
  set_zoom_supported(true);
  get_view_settings().set_zoom(2.0);
  set_grid_supported(true);
  set_layer_visibility_supported(true);
  set_entity_type_visibility_supported(true);

  // Open the file.
  model = new MapModel(quest, map_id, this);
  get_undo_stack().setClean();

  // Prepare the gui.
  const int side_width = 350;
  ui.splitter->setSizes({ side_width, width() - side_width });
  ui.map_side_splitter->setStretchFactor(0, 0);  // Don't expand the map properties view
  ui.map_side_splitter->setStretchFactor(1, 1);  // but only the tileset view.
  ui.tileset_field->set_resource_type(ResourceType::TILESET);
  ui.tileset_field->set_quest(quest);
  ui.music_field->set_resource_type(ResourceType::MUSIC);
  ui.music_field->set_quest(quest);
  ui.music_field->add_special_value("none", tr("<No music>"), 0);
  ui.music_field->add_special_value("same", tr("<Same as before>"), 1);
  ui.tileset_view->set_read_only(true);
  ui.map_view->set_model(model);
  ui.map_view->set_view_settings(get_view_settings());
  update();

  // Make connections.
  connect(&get_resources(), SIGNAL(element_description_changed(ResourceType, QString, QString)),
          this, SLOT(update_description_to_gui()));
  connect(ui.description_field, SIGNAL(editingFinished()),
          this, SLOT(set_description_from_gui()));

  connect(ui.width_field, SIGNAL(editingFinished()),
          this, SLOT(change_size_requested()));
  connect(ui.height_field, SIGNAL(editingFinished()),
          this, SLOT(change_size_requested()));
  connect(model, SIGNAL(size_changed(QSize)),
          this, SLOT(update_size_field()));

  connect(ui.world_check_box, SIGNAL(stateChanged(int)),
          this, SLOT(world_check_box_changed()));
  connect(ui.world_field, SIGNAL(editingFinished()),
          this, SLOT(change_world_requested()));
  connect(model, SIGNAL(world_changed(QString)),
          this, SLOT(update_world_field()));

  connect(ui.floor_check_box, SIGNAL(stateChanged(int)),
          this, SLOT(floor_check_box_changed()));
  connect(ui.floor_field, SIGNAL(editingFinished()),
          this, SLOT(change_floor_requested()));
  connect(model, SIGNAL(floor_changed(int)),
          this, SLOT(update_floor_field()));

  connect(ui.x_field, SIGNAL(editingFinished()),
          this, SLOT(change_location_requested()));
  connect(ui.y_field, SIGNAL(editingFinished()),
          this, SLOT(change_location_requested()));
  connect(model, SIGNAL(location_changed(QPoint)),
          this, SLOT(update_location_field()));

  connect(ui.tileset_field, SIGNAL(activated(QString)),
          this, SLOT(tileset_selector_activated()));
  connect(model, SIGNAL(tileset_id_changed(QString)),
          this, SLOT(update_tileset_field()));
  connect(model, SIGNAL(tileset_id_changed(QString)),
          this, SLOT(update_tileset_view()));
  connect(ui.tileset_edit_button, SIGNAL(clicked()),
          this, SLOT(open_tileset_requested()));

  connect(ui.music_field, SIGNAL(activated(QString)),
          this, SLOT(music_selector_activated()));
  connect(model, SIGNAL(music_id_changed(QString)),
          this, SLOT(update_music_field()));

  connect(ui.map_view, SIGNAL(move_entities_requested(QList<EntityIndex>, QPoint)),
          this, SLOT(move_entities_requested(QList<EntityIndex>, QPoint)));
}

/**
 * @brief Returns the map model being edited.
 * @return The map model.
 */
MapModel& MapEditor::get_model() {
  return *model;
}

/**
 * @brief Initializes the entity creation toolbar.
 *
 * The entity creation toolbar is not made with Qt designer because
 * - one cannot create QToolBar widgets with Qt designer,
 * - we iterate on entity types to build all of them more easily.
 */
void MapEditor::build_entity_creation_toolbar() {

  QToolBar* entity_creation_toolbar = new QToolBar(this);

  // List of types proposed in the toolbar.
  // The list is specified here manually because we want to control the order
  // and all types are not included (tiles and dynamic tiles are omitted).
  const std::vector<std::pair<EntityType, QString>> types_in_toolbar = {
    { EntityType::DESTINATION, tr("Add destination") },
    { EntityType::TELETRANSPORTER, tr("Add teletransporter") },
    { EntityType::PICKABLE, tr("Add pickable") },
    { EntityType::DESTRUCTIBLE, tr("Add destructible") },
    { EntityType::CHEST, tr("Add chest") },
    { EntityType::JUMPER, tr("Add jumper") },
    { EntityType::ENEMY, tr("Add enemy") },
    { EntityType::NPC, tr("Add non-playing character") },
    { EntityType::BLOCK, tr("Add block") },
    { EntityType::SWITCH, tr("Add switch") },
    { EntityType::WALL, tr("Add wall") },
    { EntityType::SENSOR, tr("Add sensor") },
    { EntityType::CRYSTAL, tr("Add crystal") },
    { EntityType::CRYSTAL_BLOCK, tr("Add crystal block") },
    { EntityType::SHOP_TREASURE, tr("Add shop treasure") },
    { EntityType::STREAM, tr("Add stream") },
    { EntityType::DOOR, tr("Add door") },
    { EntityType::STAIRS, tr("Add stairs") },
    { EntityType::SEPARATOR, tr("Add separator") },
    { EntityType::CUSTOM, tr("Add custom entity") }
  };

  QActionGroup* button_group = new QActionGroup(this);
  for (const auto& pair : types_in_toolbar) {
    EntityType type = pair.first;
    QString icon_name = ":/images/entity_" + EntityTraits::get_lua_name(type) + ".png";
    QAction* action = new QAction(QIcon(icon_name), pair.second, button_group);
    action->setCheckable(true);
    entity_creation_toolbar->addAction(action);
  }
  entity_creation_toolbar->setIconSize(QSize(32, 32));
  entity_creation_toolbar->setStyleSheet("spacing: 0");

  ui.entity_creation_layout->insertWidget(0, entity_creation_toolbar);
}

/**
 * @brief Creates a status bar in the map view.
 *
 * The status bar is not made with Qt designer because
 * one cannot create QStatusBar widgets with Qt designer.
 */
void MapEditor::build_status_bar() {

  QStatusBar* status_bar = new QStatusBar();
  ui.entity_creation_layout->addWidget(status_bar);

  connect(ui.map_view, &MapView::mouse_map_coordinates_changed, [=](const QPoint& xy) {

    QPoint snapped_xy(xy + QPoint(4, 4));
    snapped_xy = snapped_xy - QPoint(
          snapped_xy.x() % 8,
          snapped_xy.y() % 8);
    QString message = tr("%1,%2").arg(snapped_xy.x()).arg(snapped_xy.y());
    status_bar->showMessage(message);
  });

  connect(ui.map_view, &MapView::mouse_left, [=]() {

    status_bar->clearMessage();
  });
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
  update_size_field();
  update_world_field();
  update_floor_field();
  update_location_field();
  update_tileset_field();
  update_music_field();
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
    GuiTools::error_dialog(tr("Invalid description"));
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
 * @brief Updates the size field with the data from the model.
 */
void MapEditor::update_size_field() {

  const QSize& size = model->get_size();
  ui.width_field->setValue(size.width());
  ui.height_field->setValue(size.height());
}

/**
 * @brief Modifies the map size with new values entered by the user.
 */
void MapEditor::change_size_requested() {

  QSize size(ui.width_field->value(), ui.height_field->value());
  if (size == model->get_size()) {
    return;
  }
  try_command(new SetSizeCommand(*this, size));
}

/**
 * @brief Updates the world field with the data from the model.
 */
void MapEditor::update_world_field() {

  const QString& world = model->get_world();
  if (world.isEmpty()) {
    ui.world_check_box->setChecked(false);
    ui.world_field->setEnabled(false);
  }
  else {
    ui.world_check_box->setChecked(true);
    ui.world_field->setEnabled(true);
    ui.world_field->setText(world);
  }
}

/**
 * @brief Slot called when the user clicks the "Set a world" checkbox.
 */
void MapEditor::world_check_box_changed() {

  bool checked = ui.world_check_box->isChecked();
  if (checked) {
    ui.world_field->setEnabled(true);
    if (!model->has_world() &&
        !ui.world_field->text().isEmpty()) {
      // Use the text that was still in the disabled field.
      try_command(new SetWorldCommand(*this, ui.world_field->text()));
    }
  }
  else {
    ui.world_field->setEnabled(false);
    if (model->has_world()) {
      // Remove the world but keep the text in the field.
      try_command(new SetWorldCommand(*this, ""));
    }
  }
}

/**
 * @brief Changes the world value with the new text entered by the user.
 */
void MapEditor::change_world_requested() {

  QString world = ui.world_field->text();
  if (world == model->get_world()) {
    return;
  }
  try_command(new SetWorldCommand(*this, world));
}

/**
 * @brief Updates the floor field with the data from the model.
 */
void MapEditor::update_floor_field() {

  int floor = model->get_floor();
  if (floor == MapModel::NO_FLOOR) {
    ui.floor_check_box->setChecked(false);
    ui.floor_field->setEnabled(false);
  }
  else {
    ui.floor_check_box->setChecked(true);
    ui.floor_field->setEnabled(true);
    ui.floor_field->setValue(floor);
  }
}

/**
 * @brief Slot called when the user clicks the "Set a floor" checkbox.
 */
void MapEditor::floor_check_box_changed() {

  bool checked = ui.floor_check_box->isChecked();
  if (checked) {
    ui.floor_field->setEnabled(true);
    if (!model->has_floor()) {
      // Use the value that was still in the disabled field.
      try_command(new SetFloorCommand(*this, ui.floor_field->value()));
    }
  }
  else {
    ui.floor_field->setEnabled(false);
    if (model->has_floor()) {
      // Remove the floor but keep the value in the field.
      try_command(new SetFloorCommand(*this, MapModel::NO_FLOOR));
    }
  }
}

/**
 * @brief Changes the floor value with the new text entered by the user.
 */
void MapEditor::change_floor_requested() {

  int floor = ui.floor_field->value();
  if (floor == model->get_floor()) {
    return;
  }
  try_command(new SetFloorCommand(*this, floor));
}

/**
 * @brief Updates the location field with the data from the model.
 */
void MapEditor::update_location_field() {

  const QPoint& location = model->get_location();
  ui.x_field->setValue(location.x());
  ui.y_field->setValue(location.y());
}

/**
 * @brief Modifies the map location with new values entered by the user.
 */
void MapEditor::change_location_requested() {

  QPoint location(ui.x_field->value(), ui.y_field->value());
  if (location == model->get_location()) {
    return;
  }
  try_command(new SetLocationCommand(*this, location));
}

/**
 * @brief Updates the tileset selector with the data from the model.
 */
void MapEditor::update_tileset_field() {

  ui.tileset_field->set_selected_id(model->get_tileset_id());
}

/**
 * @brief Slot called when the user changes the tileset in the selector.
 */
void MapEditor::tileset_selector_activated() {

  const QString& old_tileset_id = model->get_tileset_id();
  const QString& new_tileset_id = ui.tileset_field->get_selected_id();
  if (new_tileset_id == old_tileset_id) {
    // No change.
    return;
  }

  try_command(new SetTilesetCommand(*this, new_tileset_id));
}

/**
 * @brief Slot called when the user wants to open the selected tileset.
 */
void MapEditor::open_tileset_requested() {

  emit open_file_requested(
        get_quest(), get_quest().get_tileset_data_file_path(model->get_tileset_id()));
}

/**
 * @brief Updates the tileset selector with the data from the model.
 */
void MapEditor::update_music_field() {

  ui.music_field->set_selected_id(model->get_music_id());
}

/**
 * @brief Slot called when the user changes the music in the selector.
 */
void MapEditor::music_selector_activated() {

  const QString& old_music_id = model->get_music_id();
  const QString& new_music_id = ui.music_field->get_selected_id();
  if (new_music_id == old_music_id) {
    // No change.
    return;
  }

  try_command(new SetMusicCommand(*this, new_music_id));
}

/**
 * @brief Updates the content of the tileset view.
 */
void MapEditor::update_tileset_view() {

  ui.tileset_view->set_model(model->get_tileset_model());
}

/**
 * @brief Slot called when the user wants to move entities.
 * @param indexes Indexes of the entities to move.
 * @param translation XY translation to make.
 */
void MapEditor::move_entities_requested(const QList<EntityIndex>& indexes, const QPoint& translation) {

  if (indexes.isEmpty()) {
    return;
  }

  try_command(new MoveEntitiesCommand(*this, indexes, translation));
}
