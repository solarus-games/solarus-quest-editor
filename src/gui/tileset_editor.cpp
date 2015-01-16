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
#include "gui/change_pattern_id_dialog.h"
#include "gui/gui_tools.h"
#include "gui/tileset_editor.h"
#include "editor_exception.h"
#include "quest.h"
#include "quest_resources.h"
#include "tileset_model.h"
#include <QColorDialog>
#include <QFile>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QRegExp>
#include <QUndoStack>
#include <QTextStream>

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
 * @brief Moving a tile pattern.
 */
class SetPatternPositionCommand : public TilesetEditorCommand {

public:

  SetPatternPositionCommand(
      TilesetEditor& editor, int index, const QPoint& position) :
    TilesetEditorCommand(editor, TilesetEditor::tr("Move pattern")),
    index(index),
    position_before(get_model().get_pattern_frame(index).topLeft()),
    position_after(position) {
  }

  virtual void undo() override {

    get_model().set_pattern_position(index, position_before);
    get_model().set_selected_index(index);
  }

  virtual void redo() override {

    get_model().set_pattern_position(index, position_after);
    get_model().set_selected_index(index);
  }

private:

  int index;
  QPoint position_before;
  QPoint position_after;
};

/**
 * @brief Changing the ground of tile patterns.
 */
class SetPatternsGroundCommand : public TilesetEditorCommand {

public:

  SetPatternsGroundCommand(
      TilesetEditor& editor, const QList<int>& indexes, Ground ground) :
    TilesetEditorCommand(editor, TilesetEditor::tr("Ground")),
    indexes(indexes),
    ground_after(ground) {

    for (int index : indexes) {
      grounds_before << get_model().get_pattern_ground(index);
    }
  }

  virtual void undo() override {

    int i = 0;
    for (int index : indexes) {
      get_model().set_pattern_ground(index, grounds_before[i]);
      ++i;
    }
    get_model().set_selected_indexes(indexes);
  }

  virtual void redo() override {

    for (int index : indexes) {
      get_model().set_pattern_ground(index, ground_after);
    }
    get_model().set_selected_indexes(indexes);
  }

private:

  QList<int> indexes;
  QList<Ground> grounds_before;
  Ground ground_after;

};

/**
 * @brief Changing the default layer of tile patterns.
 */
class SetPatternsDefaultLayerCommand : public TilesetEditorCommand {

public:

  SetPatternsDefaultLayerCommand(TilesetEditor& editor, const QList<int>& indexes, Layer layer) :
    TilesetEditorCommand(editor, TilesetEditor::tr("Default layer")),
    indexes(indexes),
    layer_after(layer) {

    for (int index : indexes) {
      layers_before << get_model().get_pattern_default_layer(index);
    }
  }

  virtual void undo() override {

    int i = 0;
    for (int index : indexes) {
      get_model().set_pattern_default_layer(index, layers_before[i]);
      ++i;
    }
    get_model().set_selected_indexes(indexes);
  }

  virtual void redo() override {

    for (int index : indexes) {
      get_model().set_pattern_default_layer(index, layer_after);
    }
    get_model().set_selected_indexes(indexes);
  }

private:

  QList<int> indexes;
  QList<Layer> layers_before;
  Layer layer_after;

};

/**
 * @brief Changing the animation property of tile patterns.
 */
class SetPatternsAnimationCommand : public TilesetEditorCommand {

public:

  SetPatternsAnimationCommand(
      TilesetEditor& editor, const QList<int>& indexes, PatternAnimation animation) :
    TilesetEditorCommand(editor, TilesetEditor::tr("Animation")),
    indexes(indexes),
    animation_after(animation) {

    for (int index : indexes) {
      animations_before << get_model().get_pattern_animation(index);
    }
  }

  virtual void undo() override {

    int i = 0;
    for (int index : indexes) {
      get_model().set_pattern_animation(index, animations_before[i]);
      ++i;
    }
    get_model().set_selected_indexes(indexes);
  }

  virtual void redo() override {

    // TODO don't do anything if one fails.
    for (int index : indexes) {
      get_model().set_pattern_animation(index, animation_after);
    }
    get_model().set_selected_indexes(indexes);
  }

private:

  QList<int> indexes;
  QList<PatternAnimation> animations_before;
  PatternAnimation animation_after;

};

/**
 * @brief Changing the animation property of tile patterns.
 */
class SetPatternsSeparationCommand : public TilesetEditorCommand {

public:

  SetPatternsSeparationCommand(
      TilesetEditor& editor, const QList<int>& indexes, PatternSeparation separation) :
    TilesetEditorCommand(editor, TilesetEditor::tr("Animation separation")),
    indexes(indexes),
    separation_after(separation) {

    for (int index : indexes) {
      separations_before << get_model().get_pattern_separation(index);
    }
  }

  virtual void undo() override {

    int i = 0;
    for (int index : indexes) {
      get_model().set_pattern_separation(index, separations_before[i]);
    }
    get_model().set_selected_indexes(indexes);
  }

  virtual void redo() override {

    for (int index : indexes) {
      get_model().set_pattern_separation(index, separation_after);
    }
    get_model().set_selected_indexes(indexes);
  }

private:

  QList<int> indexes;
  QList<PatternSeparation> separations_before;
  PatternSeparation separation_after;

};

/**
 * @brief Creating a tile pattern.
 */
class CreatePatternCommand : public TilesetEditorCommand {

public:

  CreatePatternCommand(TilesetEditor& editor, const QString& pattern_id,
                       const QRect& frame, Ground ground) :
    TilesetEditorCommand(editor, TilesetEditor::tr("Create pattern")),
    index(-1),
    pattern_id(pattern_id),
    frame(frame),
    ground(ground) {
  }

  virtual void undo() override {

    get_model().delete_pattern(index);
  }

  virtual void redo() override {

    index = get_model().create_pattern(pattern_id, frame);
    get_model().set_pattern_ground(index, ground);
    get_model().add_to_selected(index);
  }

private:

  int index;
  QString pattern_id;
  QRect frame;
  Ground ground;

};

/**
 * @brief Deleting tile patterns.
 */
class DeletePatternsCommand : public TilesetEditorCommand {

public:

  DeletePatternsCommand(TilesetEditor& editor, const QList<int>& indexes) :
    TilesetEditorCommand(editor, TilesetEditor::tr("Delete")) {

    for (int index : indexes) {
      Pattern pattern;
      pattern.id =  get_model().index_to_id(index);
      pattern.frames_bounding_box = get_model().get_pattern_frames_bounding_box(index);
      pattern.ground = get_model().get_pattern_ground(index);
      pattern.default_layer = get_model().get_pattern_default_layer(index);
      pattern.animation = get_model().get_pattern_animation(index);
      pattern.separation = get_model().get_pattern_separation(index);
      patterns << pattern;
    }
  }

  virtual void undo() override {

    for (const Pattern& pattern : patterns) {
      int index = get_model().create_pattern(pattern.id, pattern.frames_bounding_box);
      get_model().set_pattern_ground(index, pattern.ground);
      get_model().set_pattern_default_layer(index, pattern.default_layer);
      get_model().set_pattern_animation(index, pattern.animation);
      get_model().set_pattern_separation(index, pattern.separation);
    }

    QList<int> indexes;
    for (const Pattern& pattern : patterns) {
      indexes << get_model().id_to_index(pattern.id);
    }
    get_model().set_selected_indexes(indexes);
  }

  virtual void redo() override {

    for (const Pattern& pattern : patterns) {
      int index = get_model().id_to_index(pattern.id);
      get_model().delete_pattern(index);
    }
  }

private:

  struct Pattern {
    QString id;
    QRect frames_bounding_box;
    Ground ground;
    Layer default_layer;
    PatternAnimation animation;
    PatternSeparation separation;
  };

  QList<Pattern> patterns;

};

/**
 * @brief Changing the id of a tile pattern.
 */
class SetPatternIdCommand : public TilesetEditorCommand {

public:

  SetPatternIdCommand(TilesetEditor& editor, int old_index, const QString& new_id) :
    TilesetEditorCommand(editor, TilesetEditor::tr("Pattern id")),
    index_before(old_index),
    index_after(-1),
    id_before(get_model().index_to_id(old_index)),
    id_after(new_id) {
  }

  virtual void undo() override {
    get_model().set_pattern_id(index_after, id_before);
    get_model().set_selected_index(index_before);
  }

  virtual void redo() override {
    index_after = get_model().set_pattern_id(index_before, id_after);
    get_model().set_selected_index(index_after);
  }

private:

  int index_before;
  int index_after;
  QString id_before;
  QString id_after;

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
  set_zoom_supported(true);
  set_zoom(2.0);

  // Open the file.
  model = new TilesetModel(quest, tileset_id, this);
  get_undo_stack().setClean();

  // Prepare the gui.
  layout()->addWidget(ui.splitter);
  const int side_width = 400;
  ui.splitter->setSizes({ side_width, width() - side_width });
  ui.patterns_list_view->set_model(*model);
  ui.tileset_view->set_model(model);
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

  connect(ui.pattern_id_button, SIGNAL(clicked()),
          this, SLOT(change_selected_pattern_id_requested()));
  connect(ui.tileset_view, SIGNAL(change_selected_pattern_id_requested()),
          this, SLOT(change_selected_pattern_id_requested()));
  connect(ui.patterns_list_view, SIGNAL(change_selected_pattern_id_requested()),
          this, SLOT(change_selected_pattern_id_requested()));
  connect(model, SIGNAL(pattern_id_changed(int, QString, int, QString)),
          this, SLOT(update_pattern_id_field()));

  connect(ui.tileset_view, SIGNAL(change_selected_pattern_position_requested(QPoint)),
          this, SLOT(change_selected_pattern_position_requested(QPoint)));

  connect(ui.ground_field, SIGNAL(activated(QString)),
          this, SLOT(ground_selector_activated()));
  connect(ui.tileset_view, SIGNAL(change_selected_patterns_ground_requested(Ground)),
          this, SLOT(change_selected_patterns_ground_requested(Ground)));
  connect(model, SIGNAL(pattern_ground_changed(int, Ground)),
          this, SLOT(update_ground_field()));

  connect(ui.default_layer_field, SIGNAL(activated(QString)),
          this, SLOT(default_layer_selector_activated()));
  connect(ui.tileset_view, SIGNAL(change_selected_patterns_default_layer_requested(Layer)),
          this, SLOT(change_selected_patterns_default_layer_requested(Layer)));
  connect(model, SIGNAL(pattern_default_layer_changed(int, Layer)),
          this, SLOT(update_default_layer_field()));

  connect(ui.animation_type_field, SIGNAL(activated(QString)),
          this, SLOT(animation_type_selector_activated()));
  connect(ui.tileset_view, SIGNAL(change_selected_patterns_animation_requested(PatternAnimation)),
          this, SLOT(change_selected_patterns_animation_requested(PatternAnimation)));
  connect(model, SIGNAL(pattern_animation_changed(int, PatternAnimation)),
          this, SLOT(update_animation_type_field()));
  connect(model, SIGNAL(pattern_animation_changed(int, PatternAnimation)),
          this, SLOT(update_animation_separation_field()));

  connect(ui.animation_separation_field, SIGNAL(activated(QString)),
          this, SLOT(animation_separation_selector_activated()));
  connect(ui.tileset_view, SIGNAL(change_selected_patterns_separation_requested(PatternSeparation)),
          this, SLOT(change_selected_patterns_separation_requested(PatternSeparation)));
  connect(model, SIGNAL(pattern_separation_changed(int, PatternSeparation)),
          this, SLOT(update_animation_separation_field()));

  connect(model, SIGNAL(pattern_created(int, QString)),
          this, SLOT(update_num_patterns_field()));
  connect(ui.tileset_view, SIGNAL(create_pattern_requested(QString, QRect, Ground)),
          this, SLOT(create_pattern_requested(QString, QRect, Ground)));

  connect(model, SIGNAL(pattern_deleted(int, QString)),
          this, SLOT(update_num_patterns_field()));
  connect(ui.patterns_list_view, SIGNAL(delete_selected_patterns_requested()),
          this, SLOT(delete_selected_patterns_requested()));
  connect(ui.tileset_view, SIGNAL(delete_selected_patterns_requested()),
          this, SLOT(delete_selected_patterns_requested()));

  connect(&model->get_selection_model(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(update_pattern_view()));

  connect(this, SIGNAL(zoom_changed(double)),
          ui.tileset_view, SLOT(set_zoom(double)));
  connect(ui.tileset_view, SIGNAL(zoom_changed(double)),
          this, SLOT(set_zoom(double)));
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

  try_command(new SetBackgroundCommand(*this, color));
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

  // If no pattern is selected, disable the tile pattern view.
  ui.pattern_properties_group_box->setEnabled(!model->is_selection_empty());
}

/**
 * @brief Slot called when the user wants to move a tile pattern.
 */
void TilesetEditor::change_selected_pattern_position_requested(const QPoint& position) {

  int index = model->get_selected_index();
  if (index == -1) {
    // No pattern selected or several patterns selected.
    return;
  }

  try_command(new SetPatternPositionCommand(*this, index, position));
}

/**
 * @brief Updates the pattern id field from the model.
 */
void TilesetEditor::update_pattern_id_field() {

  // Get the id of the selected pattern
  // (an empty string if no pattern is selected or if multiple patterns are).
  QString pattern_id = model->index_to_id(model->get_selected_index());
  ui.pattern_id_value->setText(pattern_id);

  bool enable = !pattern_id.isEmpty();
  ui.pattern_id_label->setEnabled(enable);
  ui.pattern_id_field->setEnabled(enable);
}

/**
 * @brief Slot called when the user wants to change the id of the selected
 * pattern.
 */
void TilesetEditor::change_selected_pattern_id_requested() {

  int old_index = model->get_selected_index();
  if (old_index == -1) {
    // No pattern selected or several patterns selected.
    return;
  }

  QString old_id = model->index_to_id(old_index);
  ChangePatternIdDialog dialog(old_id, this);
  int result = dialog.exec();

  if (result != QDialog::Accepted) {
    return;
  }

  QString new_id = dialog.get_pattern_id();
  if (new_id == old_id) {
    // No change.
    return;
  }

  if (!dialog.get_update_references()) {
    // The change is only in the tileset file.
    try_command(new SetPatternIdCommand(*this, old_index, new_id));
  }
  else {
    // Also update references in existing maps
    // (not as an undoable command).

    if (get_quest().is_resource_element_open(ResourceType::MAP)) {
      // All maps needs to be closed before doing that.
      QMessageBox::warning(
            nullptr,
            dialog.windowTitle(),
            tr("Please close all maps before updating tile pattern references."));
      return;
    }

    try {
      // Do the change in the tileset.
      model->set_pattern_id(old_index, new_id);

      // Save the tileset and clear the undo history.
      save();
      get_undo_stack().clear();

      // Update all maps that use this tileset.
      change_pattern_id_in_maps(old_id, new_id);
    }
    catch (const EditorException& ex) {
      ex.show_dialog();
    }
  }
}

/**
 * @brief Replaces a pattern id by a new value in all maps that use this tileset.
 * @param old_pattern_id The pattern id to change.
 * @param new_pattern_id The new value.
 * @throws EditorException In case of error.
 */
void TilesetEditor::change_pattern_id_in_maps(
    const QString& old_pattern_id, const QString& new_pattern_id) {

  for (QString map_id : get_resources().get_elements(ResourceType::MAP)) {
    change_pattern_id_in_map(map_id, old_pattern_id, new_pattern_id);
  }
}

/**
 * @brief Replaces a pattern id by a new value in a map if it uses this tileset.
 * @param old_pattern_id The pattern id to change.
 * @param new_pattern_id The new value.
 * @throws EditorException In case of error.
 */
void TilesetEditor::change_pattern_id_in_map(
    const QString& map_id, const QString& old_pattern_id, const QString& new_pattern_id) {

  // We don't load the entire map with all its entities for performance.
  // Instead, we just find and replace the appropriate text in the map
  // data file.

  QFile file(get_quest().get_map_data_file_path(map_id));

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    throw EditorException(tr("Cannot open map file '%1'").arg(file.fileName()));
  }
  QTextStream in(&file);
  in.setCodec("UTF-8");
  QString content = in.readAll();
  file.close();

  QString tileset_line = "\n  tileset = \"" + model->get_tileset_id() + "\",\n";
  if (!content.contains(tileset_line)) {
    // This map uses another tileset: nothing to do.
    return;
  }

  QRegExp regex("\n  pattern = \"?" + QRegExp::escape(old_pattern_id) + "\"?,\n");
  QString replacement("\n  pattern = \"" + new_pattern_id + "\",\n");
  content = content.replace(regex, replacement);

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    throw EditorException(tr("Cannot open map file '%1' for writing").arg(file.fileName()));
  }
  QTextStream out(&file);
  out.setCodec("UTF-8");
  out << content;
  file.close();
}

/**
 * @brief Updates the ground selector from the model.
 */
void TilesetEditor::update_ground_field() {

  Ground ground = Ground::EMPTY;
  bool enable = model->is_common_pattern_ground(
        model->get_selected_indexes(), ground);

  ui.ground_label->setEnabled(enable);
  ui.ground_field->setEnabled(enable);

  if (enable) {
    ui.ground_field->set_selected_value(ground);
  }
}

/**
 * @brief Slot called when the user changes the ground in the selector.
 */
void TilesetEditor::ground_selector_activated() {

  if (model->is_selection_empty()) {
    return;
  }

  QList<int> indexes = model->get_selected_indexes();
  Ground new_ground = ui.ground_field->get_selected_value();
  Ground old_common_ground;
  if (model->is_common_pattern_ground(indexes, old_common_ground) &&
      new_ground == old_common_ground) {
    // No change.
    return;
  }

  try_command(new SetPatternsGroundCommand(*this, indexes, new_ground));
}

/**
 * @brief Slot called when the user changes the ground of selected patterns.
 * @param ground The new ground.
 */
void TilesetEditor::change_selected_patterns_ground_requested(Ground ground) {

  if (model->is_selection_empty()) {
    return;
  }

  try_command(new SetPatternsGroundCommand(*this, model->get_selected_indexes(), ground));
}

/**
 * @brief Updates the animation type selector from the model.
 */
void TilesetEditor::update_animation_type_field() {

  PatternAnimation animation = PatternAnimation::NONE;
  bool enable = model->is_common_pattern_animation(
        model->get_selected_indexes(), animation);

  ui.animation_label->setEnabled(enable);
  ui.animation_type_field->setEnabled(enable);

  if (enable) {
    ui.animation_type_field->set_selected_value(animation);
  }
}

/**
 * @brief Slot called when the user changes the animation kind in the selector.
 */
void TilesetEditor::animation_type_selector_activated() {

  if (model->is_selection_empty()) {
    return;
  }

  QList<int> indexes = model->get_selected_indexes();
  PatternAnimation new_animation = ui.animation_type_field->get_selected_value();
  PatternAnimation old_common_animation;
  if (model->is_common_pattern_animation(indexes, old_common_animation) &&
      new_animation == old_common_animation) {
    // No change.
    return;
  }

  if (!try_command(new SetPatternsAnimationCommand(*this,  indexes, new_animation))) {
    // In case of failure, restore the selector.
    update_animation_type_field();
  }
}

/**
 * @brief Slot called when the user changes the animation of selected patterns.
 * @param animation The new animation.
 */
void TilesetEditor::change_selected_patterns_animation_requested(PatternAnimation animation) {

  if (model->is_selection_empty()) {
    return;
  }

  try_command(new SetPatternsAnimationCommand(*this, model->get_selected_indexes(), animation));
}

/**
 * @brief Updates the animation separation selector from the model.
 */
void TilesetEditor::update_animation_separation_field() {

  PatternAnimation animation = PatternAnimation::NONE;
  bool multi_frame =
      model->is_common_pattern_animation(model->get_selected_indexes(), animation) &&
      PatternAnimationTraits::is_multi_frame(animation);

  PatternSeparation separation = PatternSeparation::HORIZONTAL;
  bool enable = multi_frame && model->is_common_pattern_separation(
        model->get_selected_indexes(), separation);

  ui.animation_separation_field->setEnabled(enable);

  if (enable) {
    ui.animation_separation_field->set_selected_value(separation);
  }

}

/**
 * @brief Slot called when the user changes the animation separation in the selector.
 */
void TilesetEditor::animation_separation_selector_activated() {

  if (model->is_selection_empty()) {
    return;
  }

  QList<int> indexes = model->get_selected_indexes();
  PatternSeparation new_separation = ui.animation_separation_field->get_selected_value();
  PatternSeparation old_common_separation;
  if (model->is_common_pattern_separation(indexes, old_common_separation) &&
      new_separation == old_common_separation) {
    // No change.
    return;
  }

  if (!try_command(new SetPatternsSeparationCommand(*this,  indexes, new_separation))) {
    // In case of failure, restore the selector.
    update_animation_separation_field();
  }
}

/**
 * @brief Slot called when the user changes the separation of selected patterns.
 * @param separation The new separation.
 */
void TilesetEditor::change_selected_patterns_separation_requested(PatternSeparation separation) {

  if (model->is_selection_empty()) {
    return;
  }

  if (!try_command(new SetPatternsSeparationCommand(*this, model->get_selected_indexes(), separation))) {
    // In case of failure, restore the selector.
    update_animation_separation_field();
  }
}

/**
 * @brief Updates the default layer selector from the model.
 */
void TilesetEditor::update_default_layer_field() {

  Layer default_layer = Solarus::LAYER_LOW;
  bool enable = model->is_common_pattern_default_layer(
        model->get_selected_indexes(), default_layer);

  ui.default_layer_label->setEnabled(enable);
  ui.default_layer_field->setEnabled(enable);

  if (enable) {
    ui.default_layer_field->set_selected_value(default_layer);
  }
}

/**
 * @brief Slot called when the user changes the layer in the selector.
 */
void TilesetEditor::default_layer_selector_activated() {

  if (model->is_selection_empty()) {
    return;
  }

  QList<int> indexes = model->get_selected_indexes();
  Layer new_default_layer = ui.default_layer_field->get_selected_value();
  Layer old_common_default_layer;
  if (model->is_common_pattern_default_layer(indexes, old_common_default_layer) &&
      new_default_layer == old_common_default_layer) {
    // No change.
    return;
  }

  try_command(new SetPatternsDefaultLayerCommand(*this, indexes, new_default_layer));
}

/**
 * @brief Slot called when the user changes the default layer of selected patterns.
 * @param default_layer The new default layer.
 */
void TilesetEditor::change_selected_patterns_default_layer_requested(Layer default_layer) {

  if (model->is_selection_empty()) {
    return;
  }

  try_command(new SetPatternsDefaultLayerCommand(*this, model->get_selected_indexes(), default_layer));
}

/**
 * @brief Slot called when the user wants to create a pattern.
 * @param paterrn_id Id of the pattern to create.
 * @param frame Position of the frame in the tileset image
 * (it will be a single-frame pattern).
 * @param ground Ground to set.
 */
void TilesetEditor::create_pattern_requested(
    const QString& pattern_id, const QRect& frame, Ground ground) {

  try_command(new CreatePatternCommand(*this, pattern_id, frame, ground));
}

/**
 * @brief Slot called when the user wants to delete the selected patterns.
 */
void TilesetEditor::delete_selected_patterns_requested() {

  QList<int> indexes = model->get_selected_indexes();

  if (indexes.empty()) {
    return;
  }

  QString question_text;
  if (indexes.size() == 1) {
    QString pattern_id = model->index_to_id(indexes.first());
    question_text = tr("Do you really want to delete pattern '%1'?").
        arg(pattern_id);
  }
  else {
    question_text = tr("Do you really want to delete these %1 patterns?").
        arg(indexes.size());
  }

  QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        tr("Delete confirmation"),
        question_text,
        QMessageBox::Yes | QMessageBox::No);

  if (answer != QMessageBox::Yes) {
    return;
  }

  try_command(new DeletePatternsCommand(*this, indexes));
}
