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
#include "gui/sprite_editor.h"
#include "editor_exception.h"
#include "quest.h"
#include "quest_resources.h"
#include "sprite_model.h"
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
  model(nullptr) {

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
  set_zoom(2.0);

  // Open the file.
  model = new SpriteModel(quest, sprite_id, this);
  get_undo_stack().setClean();

  // Prepare the gui.
  const int side_width = 400;
  ui.splitter->setSizes({ side_width, width() - side_width });
  ui.sprite_tree_view->set_model(*model);
  update();

  // Make connections.
  connect(&get_resources(),
          SIGNAL(element_description_changed(ResourceType, const QString&, const QString&)),
          this, SLOT(update_description_to_gui()));
  connect(ui.description_field, SIGNAL(editingFinished()),
          this, SLOT(set_description_from_gui()));

  connect(model, SIGNAL(animation_image_changed(Index,QString)),
          this, SLOT(update_animation_source_image_field()));

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

  connect(&model->get_selection_model(),
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(update_animation_view()));
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
 * @brief Updates everything in the gui.
 */
void SpriteEditor::update() {

  update_sprite_id_field();
  update_description_to_gui();
  update_animation_view();
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

  QSignalBlocker(this);
  try {
    get_resources().set_description(ResourceType::SPRITE, sprite_id, description);
    get_resources().save();
  }
  catch (const EditorException& ex) {
    ex.print_message();
  }
}

/**
 * @brief Fills the animation view.
 *
 * If a single animation is selected, its properties are displayed in the
 * animation view.
 * Otherwise, the animation view becomes disabled.
 */
void SpriteEditor::update_animation_view() {

  update_animation_source_image_field();
  update_animation_frame_delay_field();
  update_animation_loop_on_frame_field();

  // If no animation is selected, disable the animation view.
  bool enable = model->get_selected_index().is_valid();
  ui.animation_properties_group_box->setEnabled(enable);
}

/**
 * @brief Updates the source image field from the model.
 */
void SpriteEditor::update_animation_source_image_field() {

  QString src_image =
      model->get_animation_source_image(model->get_selected_index());
  ui.src_image_value->setText(src_image);
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
