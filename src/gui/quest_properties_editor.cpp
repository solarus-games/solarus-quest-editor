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
#include "gui/quest_properties_editor.h"
#include "gui/gui_tools.h"
#include "quest.h"
#include <QUndoStack>

namespace {

/**
 * @brief Parent class of all undoable commands of the quest properties editor.
 */
class QuestPropertiesEditorCommand : public QUndoCommand {

public:

  QuestPropertiesEditorCommand(
      QuestPropertiesEditor& editor, const QString& text) :
    QUndoCommand(text),
    editor(editor) {
  }

  QuestPropertiesEditor& get_editor() const {
    return editor;
  }

  QuestProperties& get_model() const {
    return editor.get_model();
  }

private:

  QuestPropertiesEditor& editor;

};

/**
 * @brief Change write directory.
 */
class SetWriteDirCommand : public QuestPropertiesEditorCommand {

public:

  SetWriteDirCommand(QuestPropertiesEditor& editor, const QString& write_dir) :
    QuestPropertiesEditorCommand(
      editor, QuestPropertiesEditor::tr("Change write directory")),
    write_dir_before(get_model().get_write_dir()),
    write_dir_after(write_dir) {
  }

  virtual void undo() override {

    get_model().set_write_dir(write_dir_before);
  }

  virtual void redo() override {

    get_model().set_write_dir(write_dir_after);
  }

private:

  QString write_dir_before;
  QString write_dir_after;
};

/**
 * @brief Change title bar.
 */
class SetTitleBarCommand : public QuestPropertiesEditorCommand {

public:

  SetTitleBarCommand(QuestPropertiesEditor& editor, const QString& title_bar) :
    QuestPropertiesEditorCommand(
      editor, QuestPropertiesEditor::tr("Change title bar")),
    title_bar_before(get_model().get_title_bar()),
    title_bar_after(title_bar) {
  }

  virtual void undo() override {

    get_model().set_title_bar(title_bar_before);
  }

  virtual void redo() override {

    get_model().set_title_bar(title_bar_after);
  }

private:

  QString title_bar_before;
  QString title_bar_after;
};

/**
 * @brief Change normal size.
 */
class SetNormalSizeCommand : public QuestPropertiesEditorCommand {

public:

  SetNormalSizeCommand(QuestPropertiesEditor& editor, const QSize& size) :
    QuestPropertiesEditorCommand(
      editor, QuestPropertiesEditor::tr("Change normal size")),
    size_before(get_model().get_normal_quest_size()),
    size_after(size) {
  }

  virtual void undo() override {

    get_model().set_normal_quest_size(size_before);
  }

  virtual void redo() override {

    get_model().set_normal_quest_size(size_after);
  }

private:

  QSize size_before;
  QSize size_after;
};

/**
 * @brief Change minimum size.
 */
class SetMinSizeCommand : public QuestPropertiesEditorCommand {

public:

  SetMinSizeCommand(QuestPropertiesEditor& editor, const QSize& size) :
    QuestPropertiesEditorCommand(
      editor, QuestPropertiesEditor::tr("Change minimum size")),
    normal_size_before(get_model().get_normal_quest_size()),
    min_size_before(get_model().get_min_quest_size()),
    min_size_after(size) {

    // Normal size after.
    normal_size_after = normal_size_before;
    if (normal_size_after.width() < min_size_after.width()) {
      normal_size_after.setWidth(min_size_after.width());
    }
    if (normal_size_after.height() < min_size_after.height()) {
      normal_size_after.setHeight(min_size_after.height());
    }
  }

  virtual void undo() override {

    get_model().set_min_quest_size(min_size_before);
    get_model().set_normal_quest_size(normal_size_before);
  }

  virtual void redo() override {

    get_model().set_min_quest_size(min_size_after);
    get_model().set_normal_quest_size(normal_size_after);
  }

private:

  QSize normal_size_before;
  QSize normal_size_after;
  QSize min_size_before;
  QSize min_size_after;
};

/**
 * @brief Change maximum size.
 */
class SetMaxSizeCommand : public QuestPropertiesEditorCommand {

public:

  SetMaxSizeCommand(QuestPropertiesEditor& editor, const QSize& size) :
    QuestPropertiesEditorCommand(
      editor, QuestPropertiesEditor::tr("Change maximum size")),
    normal_size_before(get_model().get_normal_quest_size()),
    max_size_before(get_model().get_max_quest_size()),
    max_size_after(size) {

    // Normal size after.
    normal_size_after = normal_size_before;
    if (normal_size_after.width() > max_size_after.width()) {
      normal_size_after.setWidth(max_size_after.width());
    }
    if (normal_size_after.height() > max_size_after.height()) {
      normal_size_after.setHeight(max_size_after.height());
    }
  }

  virtual void undo() override {

    get_model().set_max_quest_size(max_size_before);
    get_model().set_normal_quest_size(normal_size_before);
  }

  virtual void redo() override {

    get_model().set_max_quest_size(max_size_after);
    get_model().set_normal_quest_size(normal_size_after);
  }

private:

  QSize normal_size_before;
  QSize normal_size_after;
  QSize max_size_before;
  QSize max_size_after;
};

}

/**
 * @brief Creates a quest properties editor.
 * @param properties Initial value properties.
 * @param parent Parent object or nullptr.
 */
QuestPropertiesEditor::QuestPropertiesEditor(Quest &quest, QWidget* parent) :
  Editor(quest, quest.get_properties_path(), parent),
  model(quest) {

  ui.setupUi(this);
  update();

  // Editor properties.
  set_title(tr("Quest properties"));
  set_icon(QIcon(":/images/icon_solarus.png"));
  set_close_confirm_message(
        tr("Quest properties have been modified. Save changes?"));

  get_undo_stack().setClean();

  connect(&model, SIGNAL(write_dir_changed(QString)),
          this, SLOT(update_write_dir_field()));
  connect(ui.write_dir_field, SIGNAL(editing_finished()),
          this, SLOT(change_write_dir_requested()));

  connect(&model, SIGNAL(title_bar_changed(QString)),
          this, SLOT(update_title_bar_field()));
  connect(ui.title_bar_field, SIGNAL(editing_finished()),
          this, SLOT(change_title_bar_requested()));

  connect(&model, SIGNAL(normal_size_changed(QSize)),
          this, SLOT(update_normal_size_field()));
  connect(ui.normal_size_width_field, SIGNAL(editingFinished()),
          this, SLOT(change_normal_size_requested()));
  connect(ui.normal_size_height_field, SIGNAL(editingFinished()),
          this, SLOT(change_normal_size_requested()));

  connect(&model, SIGNAL(min_size_changed(QSize)),
          this, SLOT(update_min_size_field()));
  connect(ui.min_size_width_field, SIGNAL(editingFinished()),
          this, SLOT(change_min_size_requested()));
  connect(ui.min_size_height_field, SIGNAL(editingFinished()),
          this, SLOT(change_min_size_requested()));

  connect(&model, SIGNAL(max_size_changed(QSize)),
          this, SLOT(update_max_size_field()));
  connect(ui.max_size_width_field, SIGNAL(editingFinished()),
          this, SLOT(change_max_size_requested()));
  connect(ui.max_size_height_field, SIGNAL(editingFinished()),
          this, SLOT(change_max_size_requested()));
}

/**
 * @brief Returns the model.
 * @return The model.
 */
QuestProperties& QuestPropertiesEditor::get_model() {

  return model;
}

/**
 * @brief Saves the properties file.
 */
void QuestPropertiesEditor::save() {

  // Update properties of the quest
  QuestProperties& properties = get_quest().get_properties();
  properties.set_write_dir(model.get_write_dir());
  properties.set_title_bar(model.get_title_bar());
  properties.set_normal_quest_size(model.get_normal_quest_size());
  properties.set_min_quest_size(model.get_min_quest_size());
  properties.set_max_quest_size(model.get_max_quest_size());

  properties.save();
}

/**
 * @brief Update field values.
 */
void QuestPropertiesEditor::update() {

  ui.solarus_version_value->setText(model.get_solarus_version());

  update_write_dir_field();
  update_title_bar_field();
  update_normal_size_field();
  update_min_size_field();
  update_max_size_field();
}

/**
 * @brief Update the write directory field.
 */
void QuestPropertiesEditor::update_write_dir_field() {

  ui.write_dir_field->set_text(model.get_write_dir());
}

/**
 * @brief Slot called when the user change the write directory.
 */
void QuestPropertiesEditor::change_write_dir_requested() {

  QString write_dir = ui.write_dir_field->text();
  QString old_write_dir = model.get_write_dir();
  if (write_dir == old_write_dir) {
    // No change.
    return;
  }

  try_command(new SetWriteDirCommand(*this, write_dir));
}

/**
 * @brief Update the title bar field.
 */
void QuestPropertiesEditor::update_title_bar_field() {

  ui.title_bar_field->set_text(model.get_title_bar());
}

/**
 * @brief Slot called when the user change the title bar.
 */
void QuestPropertiesEditor::change_title_bar_requested() {

  QString title_bar = ui.title_bar_field->text();
  QString old_title_bar = model.get_title_bar();
  if (title_bar == old_title_bar) {
    // No change.
    return;
  }

  try_command(new SetTitleBarCommand(*this, title_bar));
}

/**
 * @brief Update the normal size field.
 */
void QuestPropertiesEditor::update_normal_size_field() {

  QSize size = model.get_normal_quest_size();
  ui.normal_size_width_field->setValue(size.width());
  ui.normal_size_height_field->setValue(size.height());
}

/**
 * @brief Slot called when the user change the normal size.
 */
void QuestPropertiesEditor::change_normal_size_requested() {

  QSize size(ui.normal_size_width_field->value(),
             ui.normal_size_height_field->value());
  QSize old_size = model.get_normal_quest_size();
  if (size == old_size) {
    // No change.
    return;
  }

  try_command(new SetNormalSizeCommand(*this, size));
}

/**
 * @brief Update the minimum size field.
 */
void QuestPropertiesEditor::update_min_size_field() {

  QSize size = model.get_min_quest_size();
  ui.min_size_width_field->setValue(size.width());
  ui.min_size_height_field->setValue(size.height());

  set_min_size_values();
}

/**
 * @brief Slot called when the user change the minimum size.
 */
void QuestPropertiesEditor::change_min_size_requested() {

  QSize size(ui.min_size_width_field->value(),
             ui.min_size_height_field->value());
  QSize old_size = model.get_min_quest_size();
  if (size == old_size) {
    // No change.
    return;
  }

  try_command(new SetMinSizeCommand(*this, size));
}

/**
 * @brief Update the maximum size field.
 */
void QuestPropertiesEditor::update_max_size_field() {

  QSize size = model.get_max_quest_size();
  ui.max_size_width_field->setValue(size.width());
  ui.max_size_height_field->setValue(size.height());

  set_max_size_values();
}

/**
 * @brief Slot called when the user change the maximum size.
 */
void QuestPropertiesEditor::change_max_size_requested() {

  QSize size(ui.max_size_width_field->value(),
             ui.max_size_height_field->value());
  QSize old_size = model.get_max_quest_size();
  if (size == old_size) {
    // No change.
    return;
  }

  try_command(new SetMaxSizeCommand(*this, size));
}

/**
 * @brief Changes the minimum values of normal and maximum size fields.
 */
void QuestPropertiesEditor::set_min_size_values() {

  QSize size(ui.min_size_width_field->value(),
              ui.min_size_height_field->value());

  ui.normal_size_width_field->setMinimum(size.width());
  ui.normal_size_height_field->setMinimum(size.height());

  ui.max_size_width_field->setMinimum(size.width());
  ui.max_size_height_field->setMinimum(size.height());
}

/**
 * @brief Changes the maximum values of normal and minimum size fields.
 */
void QuestPropertiesEditor::set_max_size_values() {

  QSize size(ui.max_size_width_field->value(),
             ui.max_size_height_field->value());

  ui.normal_size_width_field->setMaximum(size.width());
  ui.normal_size_height_field->setMaximum(size.height());

  ui.min_size_width_field->setMaximum(size.width());
  ui.min_size_height_field->setMaximum(size.height());
}
