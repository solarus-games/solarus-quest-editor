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
#include "tileset_model.h"
#include <QColorDialog>
#include <QItemSelectionModel>
#include <QUndoStack>

namespace {

/**
 * @brief Parent class of all undoable commands of the tileset editor.
 */
class TilesetEditorCommand : public QUndoCommand {

public:

  TilesetEditorCommand(TilesetEditor& editor, const QString& text) :
    QUndoCommand(text),
    editor(editor) {
  }

  TilesetEditor& get_editor() const {
    return editor;
  }

  TilesetModel& get_model() const {
    return editor.get_model();
  }

private:

  TilesetEditor& editor;

};

/**
 * @brief Changing the background color of the tileset.
 */
class SetBackgroundCommand : public TilesetEditorCommand {

public:

  SetBackgroundCommand(TilesetEditor& editor, const QColor& color) :
    TilesetEditorCommand(editor, TilesetEditor::tr("Background color")),
    color_before(get_model().get_background_color()),
    color_after(color) {
  }

  virtual void undo() override {
    get_model().set_background_color(color_before);
  }

  virtual void redo() override {
    get_model().set_background_color(color_after);
  }

private:

  QColor color_before;
  QColor color_after;

};

/**
 * @brief Changing the ground of a tile pattern.
 */
class SetPatternGroundCommand : public TilesetEditorCommand {

public:

  SetPatternGroundCommand(TilesetEditor& editor, int index, Ground ground) :
    TilesetEditorCommand(editor, TilesetEditor::tr("Ground")),
    index(index),
    ground_before(get_model().get_pattern_ground(index)),
    ground_after(ground) {
  }

  virtual void undo() override {
    get_model().set_pattern_ground(index, ground_before);
  }

  virtual void redo() override {
    get_model().set_pattern_ground(index, ground_after);
  }

private:

  int index;
  Ground ground_before;
  Ground ground_after;

};

/**
 * @brief Changing the default layer of a tile pattern.
 */
class SetPatternDefaultLayerCommand : public TilesetEditorCommand {

public:

  SetPatternDefaultLayerCommand(TilesetEditor& editor, int index, Layer layer) :
    TilesetEditorCommand(editor, TilesetEditor::tr("Default layer")),
    index(index),
    layer_before(get_model().get_pattern_default_layer(index)),
    layer_after(layer) {
  }

  virtual void undo() override {
    get_model().set_pattern_default_layer(index, layer_before);
  }

  virtual void redo() override {
    get_model().set_pattern_default_layer(index, layer_after);
  }

private:

  int index;
  Layer layer_before;
  Layer layer_after;

};

/**
 * @brief Changing the animation property of a tile pattern.
 */
class SetPatternAnimationCommand : public TilesetEditorCommand {

public:

  SetPatternAnimationCommand(
      TilesetEditor& editor, int index, TilePatternAnimation animation) :
    TilesetEditorCommand(editor, TilesetEditor::tr("Animation")),
    index(index),
    animation_before(get_model().get_pattern_animation(index)),
    animation_after(animation) {
  }

  virtual void undo() override {
    get_model().set_pattern_animation(index, animation_before);
  }

  virtual void redo() override {
    get_model().set_pattern_animation(index, animation_after);
  }

private:

  int index;
  TilePatternAnimation animation_before;
  TilePatternAnimation animation_after;

};

/**
 * @brief Changing the animation property of a tile pattern.
 */
class SetPatternSeparationCommand : public TilesetEditorCommand {

public:

  SetPatternSeparationCommand(
      TilesetEditor& editor, int index, TilePatternSeparation separation) :
    TilesetEditorCommand(editor, TilesetEditor::tr("Animation separation")),
    index(index),
    separation_before(get_model().get_pattern_separation(index)),
    separation_after(separation) {
  }

  virtual void undo() override {
    get_model().set_pattern_separation(index, separation_before);
  }

  virtual void redo() override {
    get_model().set_pattern_separation(index, separation_after);
  }

private:

  int index;
  TilePatternSeparation separation_before;
  TilePatternSeparation separation_after;

};

}

/**
 * @brief Creates a tileset editor.
 * @param quest The quest containing the file.
 * @param path Path of the tileset data file to open.
 * @param parent The parent object or nullptr.
 * @throws EditorException If the file could not be opened.
 */
TilesetEditor::TilesetEditor(Quest& quest, const QString& path, QWidget* parent) :
  Editor(quest, path, parent),
  model(nullptr) {

  ui.setupUi(this);

  // Get the tileset.
  ResourceType resource_type;
  QString tileset_id;
  quest.check_exists(path);
  if (!quest.is_resource_element(path, resource_type, tileset_id) ||
      resource_type != ResourceType::TILESET) {
    throw EditorException("File '%1' is not a tileset");
  }
  this->tileset_id = tileset_id;

  // Editor properties.
  set_title("Tileset " + get_file_name_without_extension());
  set_icon(QIcon(":/images/icon_resource_tileset.png"));
  set_close_confirm_message(
        tr("Tileset '%1' has been modified. Save changes?").arg(tileset_id));

  // Open the file.
  model = new TilesetModel(quest, tileset_id, this);
  get_undo_stack().setClean();

  // Prepare the gui.
  layout()->addWidget(ui.splitter);
  const int side_width = 400;
  ui.splitter->setSizes(QList<int>() << side_width << width() - side_width);
  ui.patterns_list_view->set_model(*model);
  ui.tileset_view->set_model(*model);
  update();

  // Make connections.
  connect(&get_resources(), SIGNAL(element_description_changed(ResourceType, const QString&, const QString&)),
          this, SLOT(update_description_to_gui()));
  connect(ui.description_field, SIGNAL(editingFinished()),
          this, SLOT(set_description_from_gui()));

  connect(ui.background_button, SIGNAL(clicked()),
          this, SLOT(background_button_clicked()));
  connect(model, SIGNAL(background_color_changed(const QColor&)),
          this, SLOT(update_background_color()));

  connect(ui.ground_field, SIGNAL(activated(QString)),
          this, SLOT(ground_selector_activated()));
  connect(model, SIGNAL(pattern_ground_changed(int, Ground)),
          this, SLOT(update_ground_field()));

  connect(ui.default_layer_field, SIGNAL(activated(QString)),
          this, SLOT(default_layer_selector_activated()));
  connect(model, SIGNAL(pattern_default_layer_changed(int, Layer)),
          this, SLOT(update_default_layer_field()));

  connect(ui.animation_type_field, SIGNAL(activated(QString)),
          this, SLOT(animation_type_selector_activated()));
  connect(model, SIGNAL(pattern_animation_changed(int, TilePatternAnimation)),
          this, SLOT(update_animation_type_field()));

  connect(ui.animation_separation_field, SIGNAL(activated(QString)),
          this, SLOT(animation_separation_selector_activated()));
  connect(model, SIGNAL(pattern_separation_changed(int, TilePatternSeparation)),
          this, SLOT(update_animation_separation_field()));

  connect(&model->get_selection(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(update_pattern_view()));
}

/**
 * @brief Returns the tileset model being edited.
 * @return The tileset model.
 */
TilesetModel& TilesetEditor::get_model() {
  return *model;
}

/**
 * @copydoc Editor::save
 */
void TilesetEditor::save() {

  model->save();
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
 * @brief Updates everything in the gui.
 */
void TilesetEditor::update() {

  update_tileset_id_field();
  update_description_to_gui();
  update_background_color();
  update_num_patterns_field();
  update_pattern_view();
}

/**
 * @brief Updates the tileset id displaying.
 */
void TilesetEditor::update_tileset_id_field() {

  ui.tileset_id_field->setText(tileset_id);
}

/**
 * @brief Updates the pattern count displaying from the model.
 */
void TilesetEditor::update_num_patterns_field() {

  ui.num_tiles_field->setText(QString::number(model->get_num_patterns()));
}


/**
 * @brief Updates the background color button from the one of the tileset.
 */
void TilesetEditor::update_background_color() {

  QString style_sheet =
      "QPushButton {\n"
      "    background-color: %1;\n"
      "    border-style: inset;\n"
      "    border-width: 2px;\n"
      "    border-color: gray;\n"
      "    min-width: 1em;\n"
      "    padding: 1px;\n"
      "}";
  QColor background_color = model->get_background_color();
  ui.background_button->setStyleSheet(
        style_sheet.arg(background_color.name()));
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
 * @brief Slot called when the user clicks the background color button.
 */
void TilesetEditor::background_button_clicked() {

  QColor color = QColorDialog::getColor(
        model->get_background_color(), this, "Select tileset background color");
  if (!color.isValid()) {
    return;
  }

  get_undo_stack().push(new SetBackgroundCommand(*this, color));
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
    GuiTools::error_dialog("Invalid description");
    update_description_to_gui();
    return;
  }

  QSignalBlocker(this);
  try {
    get_resources().set_description(ResourceType::TILESET, tileset_id, description);
    get_resources().save();
  }
  catch (const EditorException& ex) {
    ex.print_message();
  }
}

/**
 * @brief Fills the tile pattern view.
 *
 * If a single pattern is selected, its properties are displayed in the tile
 * pattern view.
 * Otherwise, the tile pattern view becomes disabled.
 */
void TilesetEditor::update_pattern_view() {

  update_pattern_id_field();
  update_ground_field();
  update_animation_type_field();
  update_animation_separation_field();
  update_default_layer_field();

  // If no pattern is selected, or more than one pattern selected,
  // disable the tile pattern view.
  const int selected_index = model->get_selected_index();
  ui.pattern_properties_group_box->setEnabled(selected_index != -1);
}

/**
 * @brief Updates the pattern id field from the model.
 */
void TilesetEditor::update_pattern_id_field() {

  // Get the id of the selected pattern
  // (an empty string if no pattern is selected or if multiple patterns are).
  QString pattern_id = model->index_to_id(model->get_selected_index());
  ui.pattern_id_value->setText(pattern_id);
}

/**
 * @brief Updates the ground selector from the model.
 */
void TilesetEditor::update_ground_field() {

  Ground ground;
  int index = model->get_selected_index();
  if (index == -1) {
    ground = Ground::EMPTY;
  }
  else {
    ground = model->get_pattern_ground(index);
  }
  ui.ground_field->set_selected_value(ground);
}

/**
 * @brief Slot called when the user changes the ground in the selector.
 */
void TilesetEditor::ground_selector_activated() {

  int index = model->get_selected_index();
  if (index == -1) {
    // No pattern selected.
    return;
  }

  Ground ground = ui.ground_field->get_selected_value();
  if (ground == model->get_pattern_ground(index)) {
    // No change.
    return;
  }

  get_undo_stack().push(new SetPatternGroundCommand(*this, index, ground));
}

/**
 * @brief Updates the animation type selector from the model.
 */
void TilesetEditor::update_animation_type_field() {

  TilePatternAnimation animation;
  int index = model->get_selected_index();
  if (index == -1) {
    animation = TilePatternAnimation::NONE;
  }
  else {
    animation = model->get_pattern_animation(index);
  }
  ui.animation_type_field->set_selected_value(animation);

  if (TilePatternAnimationTraits::is_multi_frame(animation)) {
    ui.animation_separation_field->setEnabled(true);
  }
  else {
    ui.animation_separation_field->setEnabled(false);
  }
}

/**
 * @brief Slot called when the user changes the animation kind in the selector.
 */
void TilesetEditor::animation_type_selector_activated() {

  int index = model->get_selected_index();
  if (index == -1) {
    // No pattern selected.
    return;
  }

  TilePatternAnimation animation = ui.animation_type_field->get_selected_value();
  if (animation == model->get_pattern_animation(index)) {
    // No change.
    return;
  }

  get_undo_stack().push(new SetPatternAnimationCommand(*this, index, animation));
}

/**
 * @brief Updates the animation separation selector from the model.
 */
void TilesetEditor::update_animation_separation_field() {

  TilePatternSeparation separation;
  int index = model->get_selected_index();
  if (index == -1) {
    separation = TilePatternSeparation::HORIZONTAL;
  }
  else {
    separation = model->get_pattern_separation(index);
  }
  ui.animation_separation_field->set_selected_value(separation);
}

/**
 * @brief Slot called when the user changes the animation separation in the selector.
 */
void TilesetEditor::animation_separation_selector_activated() {

  int index = model->get_selected_index();
  if (index == -1) {
    // No pattern selected.
    return;
  }

  TilePatternSeparation separation = ui.animation_separation_field->get_selected_value();
  if (separation == model->get_pattern_separation(index)) {
    // No change.
    return;
  }

  get_undo_stack().push(new SetPatternSeparationCommand(*this, index, separation));
}

/**
 * @brief Updates the default layer selector from the model.
 */
void TilesetEditor::update_default_layer_field() {

  Layer layer;
  int index = model->get_selected_index();
  if (index == -1) {
    layer = Solarus::LAYER_LOW;
  }
  else {
    layer = model->get_pattern_default_layer(index);
  }
  ui.default_layer_field->set_selected_value(layer);
}

/**
 * @brief Slot called when the user changes the layer in the selector.
 */
void TilesetEditor::default_layer_selector_activated() {

  int index = model->get_selected_index();
  if (index == -1) {
    // No pattern selected.
    return;
  }

  Layer default_layer = ui.default_layer_field->get_selected_value();
  if (default_layer == model->get_pattern_default_layer(index)) {
    // No change.
    return;
  }

  get_undo_stack().push(new SetPatternDefaultLayerCommand(*this, index, default_layer));
}
