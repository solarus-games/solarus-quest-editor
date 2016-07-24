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
#include "widgets/change_pattern_id_dialog.h"
#include "widgets/gui_tools.h"
#include "widgets/tileset_editor.h"
#include "widgets/tileset_scene.h"
#include "editor_exception.h"
#include "editor_settings.h"
#include "quest.h"
#include "quest_resources.h"
#include "refactoring.h"
#include "tileset_model.h"
#include <QGuiApplication>
#include <QColorDialog>
#include <QDebug>
#include <QFile>
#include <QFileSystemWatcher>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QRegExp>
#include <QUndoStack>
#include <QTextStream>

namespace SolarusEditor {

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

    Q_FOREACH (int index, indexes) {
      grounds_before << get_model().get_pattern_ground(index);
    }
  }

  virtual void undo() override {

    int i = 0;
    Q_FOREACH (int index, indexes) {
      get_model().set_pattern_ground(index, grounds_before[i]);
      ++i;
    }
    get_model().set_selected_indexes(indexes);
  }

  virtual void redo() override {

    Q_FOREACH (int index, indexes) {
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

  SetPatternsDefaultLayerCommand(TilesetEditor& editor, const QList<int>& indexes, int layer) :
    TilesetEditorCommand(editor, TilesetEditor::tr("Default layer")),
    indexes(indexes),
    layer_after(layer) {

    Q_FOREACH (int index, indexes) {
      layers_before << get_model().get_pattern_default_layer(index);
    }
  }

  virtual void undo() override {

    int i = 0;
    Q_FOREACH (int index, indexes) {
      get_model().set_pattern_default_layer(index, layers_before[i]);
      ++i;
    }
    get_model().set_selected_indexes(indexes);
  }

  virtual void redo() override {

    Q_FOREACH (int index, indexes) {
      get_model().set_pattern_default_layer(index, layer_after);
    }
    get_model().set_selected_indexes(indexes);
  }

private:

  QList<int> indexes;
  QList<int> layers_before;
  int layer_after;

};

/**
 * @brief Changing the repeat mode of tile patterns.
 */
class SetPatternsRepeatModeCommand : public TilesetEditorCommand {

public:

  SetPatternsRepeatModeCommand(TilesetEditor& editor, const QList<int>& indexes, TilePatternRepeatMode repeat_mode) :
    TilesetEditorCommand(editor, TilesetEditor::tr("Repeat mode")),
    indexes(indexes),
    repeat_mode_after(repeat_mode) {

    Q_FOREACH (int index, indexes) {
      repeat_modes_before << get_model().get_pattern_repeat_mode(index);
    }
  }

  virtual void undo() override {

    int i = 0;
    Q_FOREACH (int index, indexes) {
      get_model().set_pattern_repeat_mode(index, repeat_modes_before[i]);
      ++i;
    }
    get_model().set_selected_indexes(indexes);
  }

  virtual void redo() override {

    Q_FOREACH (int index, indexes) {
      get_model().set_pattern_repeat_mode(index, repeat_mode_after);
    }
    get_model().set_selected_indexes(indexes);
  }

private:

  QList<int> indexes;
  QList<TilePatternRepeatMode> repeat_modes_before;
  TilePatternRepeatMode repeat_mode_after;

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

    Q_FOREACH (int index, indexes) {
      animations_before << get_model().get_pattern_animation(index);
    }
  }

  virtual void undo() override {

    int i = 0;
    Q_FOREACH (int index, indexes) {
      get_model().set_pattern_animation(index, animations_before[i]);
      ++i;
    }
    get_model().set_selected_indexes(indexes);
  }

  virtual void redo() override {

    // TODO don't do anything if one fails.
    Q_FOREACH (int index, indexes) {
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

    Q_FOREACH (int index, indexes) {
      separations_before << get_model().get_pattern_separation(index);
    }
  }

  virtual void undo() override {

    int i = 0;
    Q_FOREACH (int index, indexes) {
      get_model().set_pattern_separation(index, separations_before[i]);
    }
    get_model().set_selected_indexes(indexes);
  }

  virtual void redo() override {

    Q_FOREACH (int index, indexes) {
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

    Q_FOREACH (int index, indexes) {
      Pattern pattern;
      pattern.id =  get_model().index_to_id(index);
      pattern.frames_bounding_box = get_model().get_pattern_frames_bounding_box(index);
      pattern.ground = get_model().get_pattern_ground(index);
      pattern.default_layer = get_model().get_pattern_default_layer(index);
      pattern.animation = get_model().get_pattern_animation(index);
      pattern.separation = get_model().get_pattern_separation(index);
      pattern.repeat_mode = get_model().get_pattern_repeat_mode(index);
      patterns << pattern;
    }
  }

  virtual void undo() override {

    Q_FOREACH (const Pattern& pattern, patterns) {
      int index = get_model().create_pattern(pattern.id, pattern.frames_bounding_box);
      get_model().set_pattern_ground(index, pattern.ground);
      get_model().set_pattern_default_layer(index, pattern.default_layer);
      get_model().set_pattern_animation(index, pattern.animation);
      get_model().set_pattern_separation(index, pattern.separation);
      get_model().set_pattern_repeat_mode(index, pattern.repeat_mode);
    }

    QList<int> indexes;
    Q_FOREACH (const Pattern& pattern, patterns) {
      indexes << get_model().id_to_index(pattern.id);
    }
    get_model().set_selected_indexes(indexes);
  }

  virtual void redo() override {

    QList<int> indexes;
    Q_FOREACH (const Pattern& pattern, patterns) {
      indexes << get_model().id_to_index(pattern.id);
    }
    get_model().delete_patterns(indexes);
  }

private:

  struct Pattern {
    QString id;
    QRect frames_bounding_box;
    Ground ground;
    int default_layer;
    PatternAnimation animation;
    PatternSeparation separation;
    TilePatternRepeatMode repeat_mode;
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
  model(nullptr),
  tileset_image_dirty(false) {

  ui.setupUi(this);

  // Get the tileset.
  ResourceType resource_type;
  QString tileset_id;
  quest.check_exists(path);
  if (!quest.is_resource_element(path, resource_type, tileset_id) ||
      resource_type != ResourceType::TILESET) {
    throw EditorException(tr("File '%1' is not a tileset").arg(path));
  }
  this->tileset_id = tileset_id;

  // Editor properties.
  set_title(tr("Tileset %1").arg(get_file_name_without_extension()));
  set_icon(QIcon(":/images/icon_resource_tileset.png"));
  set_close_confirm_message(
        tr("Tileset '%1' has been modified. Save changes?").arg(tileset_id));
  set_select_all_supported(true);
  set_zoom_supported(true);
  ViewSettings& view_settings = get_view_settings();
  view_settings.set_zoom(2.0);
  set_grid_supported(true);

  // Open the file.
  model = new TilesetModel(quest, tileset_id, this);
  get_undo_stack().setClean();

  // Prepare the gui.
  const int side_width = 400;
  ui.splitter->setSizes({ side_width, width() - side_width });
  ui.patterns_list_view->set_model(*model);
  ui.tileset_view->set_model(model);
  ui.tileset_view->set_view_settings(get_view_settings());

  load_settings();
  update();

  // Make connections.
  connect(&get_resources(), SIGNAL(element_description_changed(ResourceType, const QString&, const QString&)),
          this, SLOT(update_description_to_gui()));
  connect(ui.description_field, SIGNAL(editingFinished()),
          this, SLOT(set_description_from_gui()));

  connect(ui.background_field, SIGNAL(color_changed(QColor)),
          this, SLOT(change_background_color()));
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

  connect(ui.default_layer_field, SIGNAL(valueChanged(int)),
          this, SLOT(change_selected_patterns_default_layer_requested(int)));
  connect(ui.tileset_view, SIGNAL(change_selected_patterns_default_layer_requested(int)),
          this, SLOT(change_selected_patterns_default_layer_requested(int)));
  connect(model, SIGNAL(pattern_default_layer_changed(int, int)),
          this, SLOT(update_default_layer_field()));

  connect(ui.repeat_mode_field, SIGNAL(activated(QString)),
          this, SLOT(repeat_mode_selector_activated()));
  connect(ui.tileset_view, SIGNAL(change_selected_patterns_repeat_mode_requested(TilePatternRepeatMode)),
          this, SLOT(change_selected_patterns_repeat_mode_requested(TilePatternRepeatMode)));
  connect(model, SIGNAL(pattern_repeat_mode_changed(int, TilePatternRepeatMode)),
          this, SLOT(update_repeat_mode_field()));

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

  QFileSystemWatcher* watcher = new QFileSystemWatcher(this);
  watcher->addPath(quest.get_tileset_tiles_image_path(tileset_id));
  connect(watcher, SIGNAL(fileChanged(QString)),
          this, SLOT(tileset_image_changed()));
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

  if (model == nullptr) {
    return;
  }
  model->save();
}

/**
 * @copydoc Editor::select_all
 */
void TilesetEditor::select_all() {

  if (ui.tileset_view == nullptr) {
    return;
  }
  ui.tileset_view->select_all();
}

/**
 * @copydoc Editor::unselect_all
 */
void TilesetEditor::unselect_all() {

  if (ui.tileset_view == nullptr) {
    return;
  }
  ui.tileset_view->unselect_all();
}

/**
 * @copydoc Editor::reload_settings
 */
void TilesetEditor::reload_settings() {

  EditorSettings settings;

  TilesetScene* scene = ui.tileset_view->get_scene();
  if (scene != nullptr) {
    QBrush brush(settings.get_value_color(EditorSettings::tileset_background));
    scene->setBackgroundBrush(brush);
  }

  get_view_settings().set_grid_style(static_cast<GridStyle>(
    settings.get_value_int(EditorSettings::tileset_grid_style)));
  get_view_settings().set_grid_color(
    settings.get_value_color(EditorSettings::tileset_grid_color));
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

  ui.background_field->set_color(model->get_background_color());
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
 * @brief Slot called when the user change the background color.
 */
void TilesetEditor::change_background_color() {

  QColor old_color = model->get_background_color();
  QColor new_color = ui.background_field->get_color();

  if (new_color == old_color) {
    // No change.
    return;
  }

  try_command(new SetBackgroundCommand(*this, new_color));
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
    GuiTools::error_dialog(tr("Invalid description"));
    update_description_to_gui();
    return;
  }

  const bool was_blocked = blockSignals(true);
  try {
    get_resources().set_description(ResourceType::TILESET, tileset_id, description);
    get_resources().save();
  }
  catch (const EditorException& ex) {
    ex.print_message();
  }
  update_description_to_gui();
  blockSignals(was_blocked);
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
  update_repeat_mode_field();

  // If no pattern is selected, disable the tile pattern view.
  ui.pattern_properties_group_box->setEnabled(!model->is_selection_empty());
}

/**
 * @brief Slot called when the PNG file of the tileset has changed.
 */
void TilesetEditor::tileset_image_changed() {

  tileset_image_dirty = true;
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
    Refactoring refactoring([=]() {

      // Do the change in the tileset.
      model->set_pattern_id(old_index, new_id);

      // Save the tileset and clear the undo history.
      save();
      get_undo_stack().clear();

      // Update all maps that use this tileset.
      return change_pattern_id_in_maps(old_id, new_id);
    });
    emit refactoring_requested(refactoring);
  }
}

/**
 * @brief Replaces a pattern id by a new value in all maps that use this tileset.
 * @param old_pattern_id The pattern id to change.
 * @param new_pattern_id The new value.
 * @return The list of files that were modified.
 * @throws EditorException In case of error.
 */
QStringList TilesetEditor::change_pattern_id_in_maps(
    const QString& old_pattern_id, const QString& new_pattern_id) {

  QStringList modified_paths;
  Q_FOREACH (const QString& map_id, get_resources().get_elements(ResourceType::MAP)) {
    if (change_pattern_id_in_map(map_id, old_pattern_id, new_pattern_id)) {
      modified_paths << get_quest().get_map_data_file_path(map_id);
    }
  }
  return modified_paths;
}

/**
 * @brief Replaces a pattern id by a new value in a map if it uses this tileset.
 * @param map_id Id of the map to update.
 * @param old_pattern_id The pattern id to change.
 * @param new_pattern_id The new value.
 * @return @c true if there was a change.
 * @throws EditorException In case of error.
 */
bool TilesetEditor::change_pattern_id_in_map(
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
    return false;
  }

  QRegularExpression regex("\n  pattern = \"?" + QRegularExpression::escape(old_pattern_id) + "\"?,\n");
  QString replacement("\n  pattern = \"" + new_pattern_id + "\",\n");
  QString old_content = content;
  content.replace(regex, replacement);

  if (content == old_content) {
    // No change.
    return false;
  }

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    throw EditorException(tr("Cannot open map file '%1' for writing").arg(file.fileName()));
  }
  QTextStream out(&file);
  out.setCodec("UTF-8");
  out << content;
  file.close();
  return true;
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

  int default_layer = 0;
  bool enable = model->is_common_pattern_default_layer(
      model->get_selected_indexes(), default_layer);

  ui.default_layer_label->setEnabled(enable);
  ui.default_layer_field->setEnabled(enable);

  if (enable) {
    const bool was_blocked = ui.default_layer_field->signalsBlocked();
    ui.default_layer_field->blockSignals(true);
    ui.default_layer_field->setValue(default_layer);
    ui.default_layer_field->blockSignals(was_blocked);
  }
}

/**
 * @brief Slot called when the user changes the default layer of selected patterns.
 * @param default_layer The new default layer.
 */
void TilesetEditor::change_selected_patterns_default_layer_requested(int default_layer) {

  if (model->is_selection_empty()) {
    return;
  }

  try_command(new SetPatternsDefaultLayerCommand(*this, model->get_selected_indexes(), default_layer));
}

/**
 * @brief Updates the repeat mode selector from the model.
 */
void TilesetEditor::update_repeat_mode_field() {

  TilePatternRepeatMode repeat_mode = TilePatternRepeatMode::ALL;
  bool enable = model->is_common_pattern_repeat_mode(
      model->get_selected_indexes(), repeat_mode);

  ui.repeat_mode_label->setEnabled(enable);
  ui.repeat_mode_field->setEnabled(enable);

  if (enable) {
    ui.repeat_mode_field->set_selected_value(repeat_mode);
  }
}

/**
 * @brief Slot called when the user changes the repeat mode in the selector.
 */
void TilesetEditor::repeat_mode_selector_activated() {

  if (model->is_selection_empty()) {
    return;
  }

  QList<int> indexes = model->get_selected_indexes();
  TilePatternRepeatMode new_repeat_mode = ui.repeat_mode_field->get_selected_value();
  TilePatternRepeatMode old_common_repeat_mode;
  if (model->is_common_pattern_repeat_mode(indexes, old_common_repeat_mode) &&
      new_repeat_mode == old_common_repeat_mode) {
    // No change.
    return;
  }

  try_command(new SetPatternsRepeatModeCommand(*this, indexes, new_repeat_mode));
}

/**
 * @brief Slot called when the user changes the repeat mode of selected patterns.
 * @param repeat_mode The new repeat mode.
 */
void TilesetEditor::change_selected_patterns_repeat_mode_requested(TilePatternRepeatMode repeat_mode) {

  if (model->is_selection_empty()) {
    return;
  }

  try_command(new SetPatternsRepeatModeCommand(*this, model->get_selected_indexes(), repeat_mode));
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

/**
 * @copydoc Editor::editor_made_visible
 */
void TilesetEditor::editor_made_visible() {

  Editor::editor_made_visible();

  if (tileset_image_dirty) {
    tileset_image_dirty = false;
    QMessageBox::StandardButton answer = QMessageBox::question(
          this,
          tr("Image was modified externally"),
          tr("The tileset image was modified.\nDo you want to refresh the tileset?"),
          QMessageBox::Yes | QMessageBox::No,
          QMessageBox::Yes
          );

    if (answer == QMessageBox::QMessageBox::No) {
      return;
    }

    TilesetModel* old_model = model;

    model = new TilesetModel(get_quest(), tileset_id, this);
    ui.tileset_view->set_model(model);
    ui.patterns_list_view->set_model(*model);

    delete old_model;
  }
}

/**
 * @brief Loads settings.
 */
void TilesetEditor::load_settings() {

  ViewSettings& view = get_view_settings();
  EditorSettings settings;

  view.set_grid_visible(
    settings.get_value_bool(EditorSettings::tileset_grid_show_at_opening));
  view.set_grid_size(
    settings.get_value_size(EditorSettings::tileset_grid_size));

  reload_settings();
}

}
