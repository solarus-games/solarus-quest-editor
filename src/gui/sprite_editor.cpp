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
#include "gui/change_source_image_dialog.h"
#include "gui/gui_tools.h"
#include "gui/get_animation_name_dialog.h"
#include "gui/sprite_editor.h"
#include "gui/sprite_scene.h"
#include "editor_exception.h"
#include "point.h"
#include "quest.h"
#include "quest_resources.h"
#include "settings.h"
#include "sprite_model.h"
#include <QFileInfo>
#include <QUndoStack>

namespace {

/**
 * @brief Parent class of all undoable commands of the sprite editor.
 */
class SpriteEditorCommand : public QUndoCommand {

public:

  SpriteEditorCommand(SpriteEditor& editor, const QString& text) :
    QUndoCommand(text),
    editor(editor) {
  }

  SpriteEditor& get_editor() const {
    return editor;
  }

  SpriteModel& get_model() const {
    return editor.get_model();
  }

private:

  SpriteEditor& editor;

};

/**
 * @brief Create an animation.
 */
class CreateAnimationCommand : public SpriteEditorCommand {

public:

  CreateAnimationCommand(
      SpriteEditor& editor, const QString& name, const QString& src_img = "") :
    SpriteEditorCommand(editor, SpriteEditor::tr("Create animation")),
    name(name),
    src_img(src_img) {
  }

  virtual void undo() override {

    get_model().delete_animation(name);
  }

  virtual void redo() override {

    get_model().create_animation(name);

    if (!src_img.isEmpty()) {
      SpriteModel::Index index(name,-1);
      get_model().set_animation_source_image(index, src_img);
    }

    get_model().set_selected_animation(name);
  }

private:

  QString name;
  QString src_img;
};

/**
 * @brief Duplicate an animation.
 */
class DuplicateAnimationCommand : public SpriteEditorCommand {

public:

  DuplicateAnimationCommand(
      SpriteEditor& editor, const SpriteModel::Index& index) :
    SpriteEditorCommand(editor, SpriteEditor::tr("Duplicate animation")),
    index(index),
    new_index(index) {
  }

  virtual void undo() override {

    get_model().delete_animation(new_index);
  }

  virtual void redo() override {

    new_index.animation_name = index.animation_name + SpriteEditor::tr(" (copy)");
    const auto& animation = get_model().get_animation_data(index);
    get_model().insert_animation(new_index, animation);
    get_model().set_selected_animation(new_index);
  }

private:

  SpriteModel::Index index;
  SpriteModel::Index new_index;
};

/**
 * @brief Change animation name.
 */
class RenameAnimationCommand : public SpriteEditorCommand {

public:

  RenameAnimationCommand(
      SpriteEditor& editor, const SpriteModel::Index& index,
      const QString& new_animation_name) :
    SpriteEditorCommand(editor, SpriteEditor::tr("Change animation name")),
    index(index),
    animation_name_before(index.animation_name),
    animation_name_after(new_animation_name) {
  }

  virtual void undo() override {

    index.animation_name = animation_name_after;
    get_model().set_animation_name(index, animation_name_before);
    get_model().set_selected_index(animation_name_before);
  }

  virtual void redo() override {

    index.animation_name = animation_name_before;
    get_model().set_animation_name(index, animation_name_after);
    get_model().set_selected_index(animation_name_after);
  }

private:

  SpriteModel::Index index;
  QString animation_name_before;
  QString animation_name_after;
};

/**
 * @brief Delete an animation.
 */
class DeleteAnimationCommand : public SpriteEditorCommand {

public:

  DeleteAnimationCommand(SpriteEditor& editor, const SpriteModel::Index& index) :
    SpriteEditorCommand(editor, SpriteEditor::tr("Delete animation")),
    index(index) {
  }

  virtual void undo() override {

    get_model().insert_animation(index, data);
    if (is_default) {
      get_model().set_default_animation_name(index.animation_name);
    }
    get_model().set_selected_animation(index);
  }

  virtual void redo() override {

    is_default = get_model().get_default_animation_name() == index.animation_name;
    data = get_model().get_animation_data(index);
    get_model().delete_animation(index);
  }

private:

  SpriteModel::Index index;
  bool is_default;
  Solarus::SpriteAnimationData data;
};

/**
 * @brief Change default animation.
 */
class SetDefaultAnimationCommand : public SpriteEditorCommand {

public:

  SetDefaultAnimationCommand(
      SpriteEditor& editor, const QString& default_animation_name) :
    SpriteEditorCommand(editor, SpriteEditor::tr("Change default animation")),
    default_animation_name_before(get_model().get_default_animation_name()),
    default_animation_name_after(default_animation_name) {
  }

  virtual void undo() override {

    get_model().set_default_animation_name(default_animation_name_before);
    get_model().set_selected_animation(default_animation_name_before);
  }

  virtual void redo() override {

    get_model().set_default_animation_name(default_animation_name_after);
    get_model().set_selected_animation(default_animation_name_after);
  }

private:

  QString default_animation_name_before;
  QString default_animation_name_after;
};

/**
 * @brief Change animation source image.
 */
class SetAnimationSourceImageCommand : public SpriteEditorCommand {

public:

  SetAnimationSourceImageCommand(
      SpriteEditor& editor, const SpriteModel::Index& index,
      const QString& source_image) :
    SpriteEditorCommand(editor, SpriteEditor::tr("Change source image")),
    index(index),
    source_image_before(get_model().get_animation_source_image(index)),
    source_image_after(source_image) {
  }

  virtual void undo() override {

    get_model().set_animation_source_image(index, source_image_before);
    get_model().set_selected_animation(index);
  }

  virtual void redo() override {

    get_model().set_animation_source_image(index, source_image_after);
    get_model().set_selected_animation(index);
  }

private:

  SpriteModel::Index index;
  QString source_image_before;
  QString source_image_after;
};

/**
 * @brief Change animation frame delay.
 */
class SetAnimationFrameDelayCommand : public SpriteEditorCommand {

public:

  SetAnimationFrameDelayCommand(
      SpriteEditor& editor, const SpriteModel::Index& index,
      uint32_t frame_delay) :
    SpriteEditorCommand(editor, SpriteEditor::tr("Change frame delay")),
    index(index),
    frame_delay_before(get_model().get_animation_frame_delay(index)),
    frame_delay_after(frame_delay) {
  }

  virtual void undo() override {

    get_model().set_animation_frame_delay(index, frame_delay_before);
    get_model().set_selected_animation(index);
  }

  virtual void redo() override {

    get_model().set_animation_frame_delay(index, frame_delay_after);
    get_model().set_selected_animation(index);
  }

private:

  SpriteModel::Index index;
  uint32_t frame_delay_before;
  uint32_t frame_delay_after;
};

/**
 * @brief Change animation loop on frame.
 */
class SetAnimationLoopOnFrameCommand : public SpriteEditorCommand {

public:

  SetAnimationLoopOnFrameCommand(
      SpriteEditor& editor, const SpriteModel::Index& index, int loop_on_frame) :
    SpriteEditorCommand(editor, SpriteEditor::tr("Change loop on frame")),
    index(index),
    loop_on_frame_before(get_model().get_animation_loop_on_frame(index)),
    loop_on_frame_after(loop_on_frame) {
  }

  virtual void undo() override {

    get_model().set_animation_loop_on_frame(index, loop_on_frame_before);
    get_model().set_selected_animation(index);
  }

  virtual void redo() override {

    get_model().set_animation_loop_on_frame(index, loop_on_frame_after);
    get_model().set_selected_animation(index);
  }

private:

  SpriteModel::Index index;
  int loop_on_frame_before;
  int loop_on_frame_after;
};

/**
 * @brief Create a direction.
 */
class CreateDirectionCommand : public SpriteEditorCommand {

public:

  CreateDirectionCommand(
      SpriteEditor& editor, const SpriteModel::Index& index,
      const QRect& frame) :
    SpriteEditorCommand(editor, SpriteEditor::tr("Add direction")),
    index(index),
    frame(frame) {
  }

  virtual void undo() override {

    get_model().delete_direction(index);
  }

  virtual void redo() override {

    index.direction_nb = get_model().add_direction(index, frame);
    get_model().set_selected_index(index);
  }

private:

  SpriteModel::Index index;
  QRect frame;
};

/**
 * @brief Duplicate a direction.
 */
class DuplicateDirectionCommand : public SpriteEditorCommand {

public:

  DuplicateDirectionCommand(
      SpriteEditor& editor, const SpriteModel::Index& index,
      const QPoint& position) :
    SpriteEditorCommand(editor, SpriteEditor::tr("Duplicate direction")),
    index(index) {

    direction = get_model().get_direction_data(index);
    direction.set_xy(Point::to_solarus_point(position));
    this->index.direction_nb =
        get_model().get_animation_num_directions(index);
  }

  virtual void undo() override {

    get_model().delete_direction(index);
  }

  virtual void redo() override {

    index.direction_nb = get_model().insert_direction(index, direction);
    get_model().set_selected_index(index);
  }

private:

  SpriteModel::Index index;
  Solarus::SpriteAnimationDirectionData direction;
};

/**
 * @brief Delete a direction.
 */
class DeleteDirectionCommand : public SpriteEditorCommand {

public:

  DeleteDirectionCommand(
      SpriteEditor& editor, const SpriteModel::Index& index) :
    SpriteEditorCommand(editor, SpriteEditor::tr("Delete direction")),
    index(index) {
  }

  virtual void undo() override {

    index.direction_nb = get_model().insert_direction(index, direction);
    get_model().set_selected_index(index);
  }

  virtual void redo() override {

    direction = get_model().get_direction_data(index);
    get_model().delete_direction(index);
  }

private:

  SpriteModel::Index index;
  Solarus::SpriteAnimationDirectionData direction;
};

/**
 * @brief Change direction size.
 */
class SetDirectionSizeCommand : public SpriteEditorCommand {

public:

  SetDirectionSizeCommand(
      SpriteEditor& editor, const SpriteModel::Index& index, const QSize& size) :
    SpriteEditorCommand(editor, SpriteEditor::tr("Change direction size")),
    index(index),
    size_before(get_model().get_direction_size(index)),
    size_after(size) {
  }

  virtual void undo() override {

    get_model().set_direction_size(index, size_before);
    get_model().set_selected_index(index);
  }

  virtual void redo() override {

    get_model().set_direction_size(index, size_after);
    get_model().set_selected_index(index);
  }

private:

  SpriteModel::Index index;
  QSize size_before;
  QSize size_after;
};

/**
 * @brief Change direction position.
 */
class SetDirectionPositionCommand : public SpriteEditorCommand {

public:

  SetDirectionPositionCommand(
      SpriteEditor& editor, const SpriteModel::Index& index,
      const QPoint& position) :
    SpriteEditorCommand(editor, SpriteEditor::tr("Change direction position")),
    index(index),
    position_before(get_model().get_direction_position(index)),
    position_after(position) {
  }

  virtual void undo() override {

    get_model().set_direction_position(index, position_before);
    get_model().set_selected_index(index);
  }

  virtual void redo() override {

    get_model().set_direction_position(index, position_after);
    get_model().set_selected_index(index);
  }

private:

  SpriteModel::Index index;
  QPoint position_before;
  QPoint position_after;
};

/**
 * @brief Change direction origin.
 */
class SetDirectionOriginCommand : public SpriteEditorCommand {

public:

  SetDirectionOriginCommand(
      SpriteEditor& editor, const SpriteModel::Index& index,
      const QPoint& origin) :
    SpriteEditorCommand(editor, SpriteEditor::tr("Change direction origin")),
    index(index),
    origin_before(get_model().get_direction_origin(index)),
    origin_after(origin) {
  }

  virtual void undo() override {

    get_model().set_direction_origin(index, origin_before);
    get_model().set_selected_index(index);
  }

  virtual void redo() override {

    get_model().set_direction_origin(index, origin_after);
    get_model().set_selected_index(index);
  }

private:

  SpriteModel::Index index;
  QPoint origin_before;
  QPoint origin_after;
};

/**
 * @brief Change direction num frames.
 */
class SetDirectionNumFramesCommand : public SpriteEditorCommand {

public:

  SetDirectionNumFramesCommand(
      SpriteEditor& editor, const SpriteModel::Index& index, int num_frames) :
    SpriteEditorCommand(editor, SpriteEditor::tr("Change direction num frames")),
    index(index),
    num_frames_before(get_model().get_direction_num_frames(index)),
    num_frames_after(num_frames) {
  }

  virtual void undo() override {

    get_model().set_direction_num_frames(index, num_frames_before);
    get_model().set_selected_index(index);
  }

  virtual void redo() override {

    get_model().set_direction_num_frames(index, num_frames_after);
    get_model().set_selected_index(index);
  }

private:

  SpriteModel::Index index;
  int num_frames_before;
  int num_frames_after;
};


/**
 * @brief Change direction num columns.
 */
class SetDirectionNumColumnsCommand : public SpriteEditorCommand {

public:

  SetDirectionNumColumnsCommand(
      SpriteEditor& editor, const SpriteModel::Index& index, int num_columns) :
    SpriteEditorCommand(editor, SpriteEditor::tr("Change direction num frames")),
    index(index),
    num_columns_before(get_model().get_direction_num_columns(index)),
    num_columns_after(num_columns) {
  }

  virtual void undo() override {

    get_model().set_direction_num_columns(index, num_columns_before);
    get_model().set_selected_index(index);
  }

  virtual void redo() override {

    get_model().set_direction_num_columns(index, num_columns_after);
    get_model().set_selected_index(index);
  }

private:

  SpriteModel::Index index;
  int num_columns_before;
  int num_columns_after;
};

}

/**
 * @brief Creates a sprite editor.
 * @param quest The quest containing the file.
 * @param path Path of the sprite data file to open.
 * @param parent The parent object or nullptr.
 * @throws EditorException If the file could not be opened.
 */
SpriteEditor::SpriteEditor(Quest& quest, const QString& path, QWidget* parent) :
  Editor(quest, path, parent),
  model(nullptr),
  quest(quest),
  auto_detect_grid(false) {

  ui.setupUi(this);

  // Get the sprite.
  ResourceType resource_type;
  QString sprite_id;
  quest.check_exists(path);
  if (!quest.is_resource_element(path, resource_type, sprite_id) ||
      resource_type != ResourceType::SPRITE) {
    throw EditorException(tr("File '%1' is not a sprite").arg(path));
  }
  this->sprite_id = sprite_id;

  // Editor properties.
  set_title(tr("Sprite %1").arg(get_file_name_without_extension()));
  set_icon(QIcon(":/images/icon_resource_sprite.png"));
  set_close_confirm_message(
        tr("Sprite '%1' has been modified. Save changes?").arg(sprite_id));
  set_zoom_supported(true);
  get_view_settings().set_zoom(2.0);
  set_grid_supported(true);

  // Open the file.
  model = new SpriteModel(quest, sprite_id, this);
  get_undo_stack().setClean();

  // Prepare the gui.
  const int side_width = 400;
  const int side_height = 550;
  ui.horizontal_splitter->setSizes({ side_width, width() - side_width });
  ui.vertical_splitter->setSizes({ side_height, height() - side_height });
  ui.sprite_tree_view->set_model(model);
  ui.sprite_view->set_model(model);
  ui.sprite_view->set_view_settings(get_view_settings());
  ui.sprite_previewer->set_model(model);
  ui.tileset_field->set_resource_type(ResourceType::TILESET);
  ui.tileset_field->set_quest(quest);
  ui.tileset_field->set_selected_id(model->get_sprite_id());

  ui.size_field->config("x", 1, 99999, 8);
  ui.size_field->set_tooltips(
    tr("Width of each frame in the image"),
    tr("Height of each frame in the image"));

  ui.position_field->config(",", 0, 99999, 8);
  ui.position_field->set_tooltips(
    tr("X coordinate of the top-left corner of area containing the frames in the image"),
    tr("Y coordinate of the top-left corner of area containing the frames in the image"));

  ui.origin_field->config("x", -9999, 99999, 4);
  ui.origin_field->set_tooltips(
    tr("X coordinate of the origin point of the sprite"),
    tr("Y coordinate of the origin point of the sprite"));

  // Select the default animation and his first direction if exists
  SpriteModel::Index index(model->get_default_animation_name());
  int num_directions = model->get_animation_num_directions(index);
  if (num_directions > 0) {
    index.direction_nb = 0;
  }
  model->set_selected_index(index);

  create_animation = create_context_menu.addAction(tr("Create animation"));
  create_direction = create_context_menu.addAction(tr("Create direction"));

  load_settings();
  update();

  // Make connections.
  connect(&get_resources(),
          SIGNAL(element_description_changed(ResourceType, const QString&, const QString&)),
          this, SLOT(update_description_to_gui()));
  connect(ui.description_field, SIGNAL(editingFinished()),
          this, SLOT(set_description_from_gui()));

  connect(model, SIGNAL(animation_image_changed(Index,QString)),
          this, SLOT(update_animation_source_image_field()));

  connect(ui.src_image_button, SIGNAL(clicked()),
          this, SLOT(change_animation_source_image_requested()));
  connect(ui.src_image_refresh_button, SIGNAL(clicked(bool)),
          this, SLOT(refresh_animation_source_image()));

  connect(ui.tileset_field, SIGNAL(activated(QString)),
          this, SLOT(tileset_selector_activated()));

  connect(model, SIGNAL(default_animation_changed(QString,QString)),
          this, SLOT(update_default_animation_field()));
  connect(ui.default_animation_value, SIGNAL(clicked()),
          this, SLOT(change_default_animation_requested()));

  connect(model, SIGNAL(animation_frame_delay_changed(Index,uint32_t)),
          this, SLOT(update_animation_frame_delay_field()));
  connect(ui.frame_delay_field, SIGNAL(editingFinished()),
          this, SLOT(change_animation_frame_delay_requested()));

  connect(model, SIGNAL(animation_loop_on_frame_changed(Index,int)),
          this, SLOT(update_animation_loop_on_frame_field()));
  connect(ui.loop_on_frame_check_box, SIGNAL(clicked()),
          this, SLOT(change_animation_loop_on_frame_requested()));
  connect(ui.loop_on_frame_field, SIGNAL(editingFinished()),
          this, SLOT(change_animation_loop_on_frame_requested()));

  connect(model, SIGNAL(direction_size_changed(Index,QSize)),
          this, SLOT(update_direction_size_field()));
  connect(ui.size_field, SIGNAL(editing_finished()),
          this, SLOT(change_direction_size_requested()));

  connect(model, SIGNAL(direction_position_changed(Index,QPoint)),
          this, SLOT(update_direction_position_field()));
  connect(ui.position_field, SIGNAL(editing_finished()),
          this, SLOT(change_direction_position_requested_from_field()));
  connect(ui.sprite_view, SIGNAL(change_selected_direction_position_requested(QPoint)),
          this, SLOT(change_direction_position_requested(QPoint)));

  connect(model, SIGNAL(direction_origin_changed(Index,QPoint)),
          this, SLOT(update_direction_origin_field()));
  connect(ui.origin_field, SIGNAL(editing_finished()),
          this, SLOT(change_direction_origin_requested()));

  connect(model, SIGNAL(direction_num_frames_changed(Index,int)),
          this, SLOT(update_direction_num_frames_field()));
  connect(ui.num_frames_field, SIGNAL(editingFinished()),
          this, SLOT(change_direction_num_frames_requested()));

  connect(model, SIGNAL(direction_num_columns_changed(Index,int)),
          this, SLOT(update_direction_num_columns_field()));
  connect(ui.num_columns_field, SIGNAL(editingFinished()),
          this, SLOT(change_direction_num_columns_requested()));

  connect(ui.create_button, SIGNAL(clicked()),
          this, SLOT(create_requested()));
  connect(create_animation, SIGNAL(triggered()),
          this, SLOT(create_animation_requested()));
  connect(create_direction, SIGNAL(triggered()),
          this, SLOT(create_direction_requested()));

  connect(ui.sprite_view, SIGNAL(add_direction_requested(QRect)),
          this, SLOT(add_direction_requested(QRect)));
  connect(ui.sprite_view, SIGNAL(duplicate_selected_direction_requested(QPoint)),
          this, SLOT(duplicate_selected_direction_requested(QPoint)));
  connect(ui.rename_button, SIGNAL(clicked()),
          this, SLOT(rename_animation_requested()));
  connect(ui.duplicate_button, SIGNAL(clicked()),
          this, SLOT(duplicate_requested()));
  connect(ui.delete_button, SIGNAL(clicked()), this, SLOT(delete_requested()));
  connect(ui.sprite_view, SIGNAL(delete_selected_direction_requested()),
          this, SLOT(delete_direction_requested()));

  connect(ui.sprite_tree_view, SIGNAL(create_animation_requested()),
          this, SLOT(create_animation_requested()));
  connect(ui.sprite_tree_view, SIGNAL(create_direction_requested()),
          this, SLOT(create_direction_requested()));
  connect(ui.sprite_tree_view, SIGNAL(rename_animation_requested()),
          this, SLOT(rename_animation_requested()));
  connect(ui.sprite_tree_view, SIGNAL(duplicate_requested()),
          this, SLOT(duplicate_requested()));
  connect(ui.sprite_tree_view, SIGNAL(delete_requested()),
          this, SLOT(delete_requested()));

  connect(&model->get_selection_model(),
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(update_selection()));
}

SpriteEditor::~SpriteEditor() {
  if (model != nullptr) {
    delete model;
  }
}

/**
 * @brief Returns the sprite model being edited.
 * @return The sprite model.
 */
SpriteModel& SpriteEditor::get_model() {
  return *model;
}

/**
 * @copydoc Editor::save
 */
void SpriteEditor::save() {

  model->save();
}

/**
 * @copydoc Editor::reload_settings
 */
void SpriteEditor::reload_settings() {

  Settings settings;

  SpriteScene* scene = ui.sprite_view->get_scene();
  if (scene != nullptr) {
    QBrush brush(settings.get_value_color(Settings::sprite_main_background));
    scene->setBackgroundBrush(brush);
  }

  get_view_settings().set_grid_style(static_cast<GridStyle>(
    settings.get_value_int(Settings::sprite_grid_style)));
  get_view_settings().set_grid_color(
    settings.get_value_color(Settings::sprite_grid_color));

  ui.sprite_previewer->set_background_color(
    settings.get_value_color(Settings::sprite_previewer_background));
  ui.sprite_previewer->set_origin_color(
    settings.get_value_color(Settings::sprite_origin_color));

  auto_detect_grid = settings.get_value_bool(Settings::sprite_auto_detect_grid);
}

/**
 * @brief Updates everything in the gui.
 */
void SpriteEditor::update() {

  update_sprite_id_field();
  update_description_to_gui();
  update_selection();
}

/**
 * @brief Updates the sprite id displaying.
 */
void SpriteEditor::update_sprite_id_field() {

  ui.sprite_id_field->setText(sprite_id);
}

/**
 * @brief Updates the content of the sprite description text edit.
 */
void SpriteEditor::update_description_to_gui() {

  QString description = get_resources().get_description(ResourceType::SPRITE, sprite_id);
  if (ui.description_field->text() != description) {
    ui.description_field->setText(description);
  }
}

/**
 * @brief Modifies the sprite description in the quest resource list with
 * the new text entered by the user.
 *
 * If the new description is invalid, an error dialog is shown.
 */
void SpriteEditor::set_description_from_gui() {

  QString description = ui.description_field->text();
  if (description == get_resources().get_description(ResourceType::SPRITE, sprite_id)) {
    return;
  }

  if (description.isEmpty()) {
    GuiTools::error_dialog(tr("Invalid description"));
    update_description_to_gui();
    return;
  }

  const bool was_blocked = blockSignals(true);
  try {
    get_resources().set_description(ResourceType::SPRITE, sprite_id, description);
    get_resources().save();
  }
  catch (const EditorException& ex) {
    ex.print_message();
  }
  update_description_to_gui();
  blockSignals(was_blocked);
}

/**
 * @brief Updates the selection.
 */
void SpriteEditor::update_selection() {

  update_animation_view();
  update_direction_view();
  auto_detect_grid_size();

  // Ensures that the selected item is visible in the tree view
  SpriteModel::Index index = model->get_selected_index();
  if (index.is_valid()) {
    ui.sprite_tree_view->scrollTo(model->get_model_index(index));
  }

  // Update buttons
  bool enable = model->get_selected_index().is_valid();
  ui.rename_button->setEnabled(enable);
  ui.delete_button->setEnabled(enable);
}

/**
 * @brief Slot called when the user wants to create an animation or a direction.
 */
void SpriteEditor::create_requested() {

  SpriteModel::Index index = model->get_selected_index();
  create_direction->setEnabled(index.is_valid());

  QPoint pos = mapToGlobal(ui.create_button->pos());
  pos.setX(pos.x() + ui.create_button->width());

  create_context_menu.popup(pos);
}

/**
 * @brief Slot called when the user wants to create an animation.
 */
void SpriteEditor::create_animation_requested() {

  GetAnimationNameDialog dialog(*model, this);

  int result = dialog.exec();
  if (result != QDialog::Accepted) {
    return;
  }

  // Try to guess to source image
  QString src_img = model->get_sprite_id() + ".png";
  QFileInfo file_info(get_quest().get_sprite_image_path(src_img));
  if (!file_info.exists()) {
    src_img = "";
  }

  QString name = dialog.get_animation_name();
  try_command(new CreateAnimationCommand(*this, name, src_img));
}

/**
 * @brief Slot called when the user wants to rename an animation.
 */
void SpriteEditor::rename_animation_requested() {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_valid()) {
    // No selection.
    return;
  }

  QString old_name = index.animation_name;
  GetAnimationNameDialog dialog(*model, old_name, this);

  int result = dialog.exec();
  if (result != QDialog::Accepted) {
    return;
  }

  QString new_name = dialog.get_animation_name();

  if (new_name == old_name) {
    return;
  }

  try_command(new RenameAnimationCommand(*this, index, new_name));
}

/**
 * @brief Slot called when the user wants to create a new direction.
 */
void SpriteEditor::create_direction_requested() {

  add_direction_requested(QRect(0, 0, 16, 16));
}

/**
 * @brief Slot called when the user wants to add a new direction.
 */
void SpriteEditor::add_direction_requested(const QRect& frame) {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_valid()) {
    // No selection.
    return;
  }

  try_command(new CreateDirectionCommand(*this, index, frame));
}

/**
 * @brief Slot called when the user wants to duplicate the selection.
 */
void SpriteEditor::duplicate_requested() {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_valid()) {
    // No selection.
    return;
  }

  if (index.is_animation_index()) {
    try_command(new DuplicateAnimationCommand(*this, index));
  } else {
    QPoint position = model->get_direction_position(index);
    try_command(new DuplicateDirectionCommand(*this, index, position));
  }
}

/**
 * @brief Slot called when the user wants to duplicate the selected direction.
 */
void SpriteEditor::duplicate_selected_direction_requested(const QPoint& position) {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_direction_index()) {
    // No selection.
    return;
  }
  try_command(new DuplicateDirectionCommand(*this, index, position));
}

/**
 * @brief Slot called when the user wants to delete an animation or a direction.
 */
void SpriteEditor::delete_requested() {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_valid()) {
    // No selection.
    return;
  }

  if (index.is_animation_index()) {
    try_command(new DeleteAnimationCommand(*this, index));
  } else {
    try_command(new DeleteDirectionCommand(*this, index));
  }
}

/**
 * @brief Slot called when the user wants to delete a direction.
 */
void SpriteEditor::delete_direction_requested() {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_direction_index()) {
    // No selection.
    return;
  }
  try_command(new DeleteDirectionCommand(*this, index));
}

/**
 * @brief Fills the animation view.
 *
 * If a single animation is selected, its properties are displayed in the
 * animation view.
 * Otherwise, the animation view becomes disabled.
 */
void SpriteEditor::update_animation_view() {

  update_default_animation_field();
  update_animation_source_image_field();
  update_animation_frame_delay_field();
  update_animation_loop_on_frame_field();

  // If no animation is selected, disable the animation view.
  bool enable = model->get_selected_index().is_valid();
  ui.animation_properties_group_box->setEnabled(enable);
}

/**
 * @brief Updates the default animation field from the model.
 */
void SpriteEditor::update_default_animation_field() {

  bool is_default = false;
  SpriteModel::Index index = model->get_selected_index();

  if (index.is_valid()) {
    if (index.animation_name == model->get_default_animation_name()) {
      is_default = true;
    }
  }

  ui.default_animation_value->setChecked(is_default);
}

/**
 * @brief Slot called when the user wants to change the default animation.
 */
void SpriteEditor::change_default_animation_requested() {

  if (!ui.default_animation_value->isChecked()) {
    ui.default_animation_value->setChecked(true);
    return;
  }

  QString old_default_animation_name = model->get_default_animation_name();
  QString new_default_animation_name =
      model->get_selected_index().animation_name;

  if (new_default_animation_name == old_default_animation_name) {
    return;
  }

  try_command(
        new SetDefaultAnimationCommand(*this, new_default_animation_name));
}

/**
 * @brief Updates the source image field from the model.
 */
void SpriteEditor::update_animation_source_image_field() {

  QString src_image =
      model->get_animation_source_image(model->get_selected_index());

  bool is_tileset = src_image == "tileset";

  ui.src_image_value->setText(src_image);
  ui.src_image_value->setVisible(!is_tileset);

  ui.tileset_field->setVisible(is_tileset);
}

/**
 * @brief Slot called when the user wants to change the animation source image.
 */
void SpriteEditor::change_animation_source_image_requested() {

  SpriteModel::Index index = model->get_selected_index();
  QString old_source_image = model->get_animation_source_image(index);

  ChangeSourceImageDialog dialog(quest, old_source_image, this);

  int result = dialog.exec();
  if (result != QDialog::Accepted) {
    return;
  }

  QString source_image = dialog.get_source_image();
  if (source_image == old_source_image) {
    return;
  }

  try_command(new SetAnimationSourceImageCommand(*this, index, source_image));
}

/**
 * @brief Slot called when the user wants to refresh the animation source image.
 */
void SpriteEditor::refresh_animation_source_image() {

  SpriteModel::Index index = model->get_selected_index();
  QString source_image = model->get_animation_source_image(index);

  model->set_animation_source_image(index, "");
  model->set_animation_source_image(index, source_image);
}

/**
 * @brief Slot called when the user changes the tileset in the selector.
 */
void SpriteEditor::tileset_selector_activated() {

  model->set_tileset_id(ui.tileset_field->get_selected_id());
}

/**
 * @brief Updates the frame delay field from the model.
 */
void SpriteEditor::update_animation_frame_delay_field() {

  uint32_t frame_delay =
      model->get_animation_frame_delay(model->get_selected_index());
  ui.frame_delay_field->setValue(frame_delay);
}

/**
 * @brief Slot called when the user wants to change the animation frame delay.
 */
void SpriteEditor::change_animation_frame_delay_requested() {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_valid()) {
    // No animation selected or several animations selected.
    return;
  }

  int frame_delay = ui.frame_delay_field->value();
  int old_frame_delay = model->get_animation_frame_delay(index);
  if (frame_delay == old_frame_delay) {
    // No change.
    return;
  }

  try_command(new SetAnimationFrameDelayCommand(*this, index, (uint32_t)frame_delay));
}

/**
 * @brief Updates the loop on frame field from the model.
 */
void SpriteEditor::update_animation_loop_on_frame_field() {

  int loop_on_frame =
      model->get_animation_loop_on_frame(model->get_selected_index());

  bool enable = loop_on_frame >= 0;

  ui.loop_on_frame_check_box->setChecked(enable);
  ui.loop_on_frame_field->setEnabled(enable);
  ui.loop_on_frame_field->setValue(loop_on_frame);
}

/**
 * @brief Slot called when the user wants to change the animation loop on frame.
 */
void SpriteEditor::change_animation_loop_on_frame_requested() {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_valid()) {
    // No animation selected or several animations selected.
    return;
  }

  int old_loop_on_frame = model->get_animation_loop_on_frame(index);

  int loop_on_frame;
  if (ui.loop_on_frame_check_box->isChecked()) {
    loop_on_frame = std::max(ui.loop_on_frame_field->value(), 0);
  } else {
    loop_on_frame = -1;
  }

  if (loop_on_frame == old_loop_on_frame) {
    // No change.
    return;
  }

  try_command(new SetAnimationLoopOnFrameCommand(*this, index, loop_on_frame));
}

/**
 * @brief Fills the direction view.
 *
 * If a direction is selected, its properties are displayed in the
 * direction view.
 * Otherwise, the direction view becomes disabled.
 */
void SpriteEditor::update_direction_view() {

  update_direction_size_field();
  update_direction_position_field();
  update_direction_origin_field();
  update_direction_num_frames_field();
  update_direction_num_columns_field();

  // If no directin is selected, disable the direction view.
  SpriteModel::Index index = model->get_selected_index();
  bool enable = index.is_direction_index();
  ui.direction_properties_group_box->setEnabled(enable);

  // expand the selected animation item
  if (enable) {
    index.direction_nb = -1;
    ui.sprite_tree_view->expand(model->get_model_index(index));
  }
}

/**
 * @brief Updates the direction size field from the model.
 */
void SpriteEditor::update_direction_size_field() {

  ui.size_field->set_size(
    model->get_direction_size(model->get_selected_index()));
}

/**
 * @brief Slot called when the user wants to change the direction size.
 */
void SpriteEditor::change_direction_size_requested() {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_direction_index()) {
    // No direction selected.
    return;
  }

  QSize old_size = model->get_direction_size(index);
  QSize size = ui.size_field->get_size();

  if (size == old_size) {
    // No change.
    return;
  }

  try_command(new SetDirectionSizeCommand(*this, index, size));
}

/**
 * @brief Updates the direction position field from the model.
 */
void SpriteEditor::update_direction_position_field() {

  ui.position_field->set_point(
    model->get_direction_position(model->get_selected_index()));
}

/**
 * @brief Slot called when the user wants to change the direction position.
 *
 * Change the direction position from x and y field.
 */
void SpriteEditor::change_direction_position_requested_from_field() {

  change_direction_position_requested(ui.position_field->get_point());
}

/**
 * @brief Slot called when the user wants to change the direction position.
 */
void SpriteEditor::change_direction_position_requested(
    const QPoint& position) {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_direction_index()) {
    // No direction selected.
    return;
  }

  QPoint old_position = model->get_direction_position(index);

  if (position == old_position) {
    // No change.
    return;
  }

  try_command(new SetDirectionPositionCommand(*this, index, position));
}

/**
 * @brief Updates the direction origin field from the model.
 */
void SpriteEditor::update_direction_origin_field() {

  ui.origin_field->set_point(
    model->get_direction_origin(model->get_selected_index()));
}

/**
 * @brief Slot called when the user wants to change the direction origin.
 */
void SpriteEditor::change_direction_origin_requested() {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_direction_index()) {
    // No direction selected.
    return;
  }

  QPoint old_origin = model->get_direction_origin(index);
  QPoint origin = ui.origin_field->get_point();

  if (origin == old_origin) {
    // No change.
    return;
  }

  try_command(new SetDirectionOriginCommand(*this, index, origin));
}

/**
 * @brief Updates the direction num frames field from the model.
 */
void SpriteEditor::update_direction_num_frames_field() {

  int num_frames = model->get_direction_num_frames(model->get_selected_index());
  ui.num_frames_field->setValue(num_frames);
}

/**
 * @brief Slot called when the user wants to change the direction num frames.
 */
void SpriteEditor::change_direction_num_frames_requested() {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_direction_index()) {
    // No direction selected.
    return;
  }

  int old_num_frames = model->get_direction_num_frames(index);
  int num_frames = ui.num_frames_field->value();

  if (num_frames == old_num_frames) {
    // No change.
    return;
  }

  try_command(new SetDirectionNumFramesCommand(*this, index, num_frames));
}

/**
 * @brief Updates the direction num columns field from the model.
 */
void SpriteEditor::update_direction_num_columns_field() {

  int num_columns = model->get_direction_num_columns(model->get_selected_index());
  ui.num_columns_field->setValue(num_columns);
}

/**
 * @brief Slot called when the user wants to change the direction num columns.
 */
void SpriteEditor::change_direction_num_columns_requested() {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_direction_index()) {
    // No direction selected.
    return;
  }

  int old_num_columns = model->get_direction_num_columns(index);
  int num_columns = ui.num_columns_field->value();

  if (num_columns == old_num_columns) {
    // No change.
    return;
  }

  try_command(new SetDirectionNumColumnsCommand(*this, index, num_columns));
}

/**
 * @brief Loads settings.
 */
void SpriteEditor::load_settings() {

  ViewSettings& view = get_view_settings();
  Settings settings;

  view.set_grid_visible(
    settings.get_value_bool(Settings::sprite_grid_show_at_opening));
  view.set_grid_size(settings.get_value_size(Settings::sprite_grid_size));

  ui.sprite_previewer->set_show_origin(
    settings.get_value_bool(Settings::sprite_origin_show_at_opening));

  reload_settings();
}

/**
 * @brief Auto detect the grid size.
 */
void SpriteEditor::auto_detect_grid_size() {

  if (auto_detect_grid) {

    SpriteModel::Index index = model->get_selected_index();

    // Try to get the first direction if no direction is selected.
    if (!index.is_direction_index()) {
      index.direction_nb = 0;
      if (!model->direction_exists(index)) {
        return;
      }
    }

    get_view_settings().set_grid_size(
      model->get_direction_first_frame_rect(index).size());
  }
}
