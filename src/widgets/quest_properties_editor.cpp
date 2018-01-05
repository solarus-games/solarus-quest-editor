/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
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
#include "widgets/quest_properties_editor.h"
#include "widgets/gui_tools.h"
#include "quest.h"
#include <QRegularExpressionValidator>
#include <QUndoStack>

namespace SolarusEditor {

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
 * @brief Change title.
 */
class SetTitleCommand : public QuestPropertiesEditorCommand {

public:

  SetTitleCommand(QuestPropertiesEditor& editor, const QString& title) :
    QuestPropertiesEditorCommand(
      editor, QuestPropertiesEditor::tr("Change title")),
    title_before(get_model().get_title()),
    title_after(title) {
  }

  virtual void undo() override {

    get_model().set_title(title_before);
  }

  virtual void redo() override {

    get_model().set_title(title_after);
  }

private:

  QString title_before;
  QString title_after;
};

/**
 * @brief Changing the short description.
 */
class SetShortDescriptionCommand : public QuestPropertiesEditorCommand {

public:
  SetShortDescriptionCommand(QuestPropertiesEditor& editor, const QString& short_description) :
    QuestPropertiesEditorCommand(
      editor, QuestPropertiesEditor::tr("Change summary")),
    short_description_before(get_model().get_short_description()),
    short_description_after(short_description) {
  }

  void undo() override {
    get_model().set_short_description(short_description_before);
  }

  void redo() override {
    get_model().set_short_description(short_description_after);
  }

private:
  QString short_description_before;
  QString short_description_after;
};

/**
 * @brief Changing the long description.
 */
class SetLongDescriptionCommand : public QuestPropertiesEditorCommand {

public:
  SetLongDescriptionCommand(QuestPropertiesEditor& editor, const QString& long_description) :
    QuestPropertiesEditorCommand(
      editor, QuestPropertiesEditor::tr("Change detailed description")),
    long_description_before(get_model().get_long_description()),
    long_description_after(long_description) {
  }

  void undo() override {
    get_model().set_long_description(long_description_before);
  }

  void redo() override {
    get_model().set_long_description(long_description_after);
  }

private:
  QString long_description_before;
  QString long_description_after;
};

/**
 * @brief Changing the author.
 */
class SetAuthorCommand : public QuestPropertiesEditorCommand {

public:
  SetAuthorCommand(QuestPropertiesEditor& editor, const QString& author) :
    QuestPropertiesEditorCommand(
      editor, QuestPropertiesEditor::tr("Change author")),
    author_before(get_model().get_author()),
    author_after(author) {
  }

  void undo() override {
    get_model().set_author(author_before);
  }

  void redo() override {
    get_model().set_author(author_after);
  }

private:
  QString author_before;
  QString author_after;
};

/**
 * @brief Changing the quest version.
 */
class SetQuestVersionCommand : public QuestPropertiesEditorCommand {

public:
  SetQuestVersionCommand(QuestPropertiesEditor& editor, const QString& quest_version) :
    QuestPropertiesEditorCommand(
      editor, QuestPropertiesEditor::tr("Change quest version")),
    quest_version_before(get_model().get_quest_version()),
    quest_version_after(quest_version) {
  }

  void undo() override {
    get_model().set_quest_version(quest_version_before);
  }

  void redo() override {
    get_model().set_quest_version(quest_version_after);
  }

private:
  QString quest_version_before;
  QString quest_version_after;
};

/**
 * @brief Changing the release date.
 */
class SetReleaseDateCommand : public QuestPropertiesEditorCommand {

public:
  SetReleaseDateCommand(QuestPropertiesEditor& editor, const QDate& release_date) :
    QuestPropertiesEditorCommand(
      editor, QuestPropertiesEditor::tr("Change release date")),
    release_date_before(get_model().get_release_date()),
    release_date_after(release_date) {
  }

  void undo() override {
    get_model().set_release_date(release_date_before);
  }

  void redo() override {
    get_model().set_release_date(release_date_after);
  }

private:
  QDate release_date_before;
  QDate release_date_after;
};

/**
 * @brief Changing the website.
 */
class SetWebsiteCommand : public QuestPropertiesEditorCommand {

public:
  SetWebsiteCommand(QuestPropertiesEditor& editor, const QString& website) :
    QuestPropertiesEditorCommand(
      editor, QuestPropertiesEditor::tr("Change website")),
    website_before(get_model().get_website()),
    website_after(website) {
  }

  void undo() override {
    get_model().set_website(website_before);
  }

  void redo() override {
    get_model().set_website(website_after);
  }

private:
  QString website_before;
  QString website_after;
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
 * @param quest The quest.
 * @param parent Parent object or nullptr.
 */
QuestPropertiesEditor::QuestPropertiesEditor(Quest& quest, QWidget* parent) :
  Editor(quest, quest.get_properties_path(), parent),
  model(quest) {

  ui.setupUi(this);
  ui.release_date_field->setDate(QDate::currentDate());

  // Don't allow slashes or backslashes in the write dir field.
  ui.write_dir_field->setValidator(
        new QRegularExpressionValidator(QRegularExpression(R"(^[^/\\]*$)"), this));

  update();

  // Editor properties.
  set_title(tr("Quest properties"));
  set_icon(QIcon(":/images/icon_solarus.png"));
  set_close_confirm_message(
        tr("Quest properties have been modified. Save changes?"));

  get_undo_stack().setClean();

  connect(&model, SIGNAL(write_dir_changed(QString)),
          this, SLOT(update_write_dir_field()));
  connect(ui.write_dir_field, SIGNAL(editingFinished()),
          this, SLOT(change_write_dir_requested()));

  connect(&model, SIGNAL(title_changed(QString)),
          this, SLOT(update_title_field()));
  connect(ui.title_field, SIGNAL(editingFinished()),
          this, SLOT(change_title_requested()));

  connect(&model, SIGNAL(short_description_changed(QString)),
          this, SLOT(update_short_description_field()));
  connect(ui.short_description_field, SIGNAL(editingFinished()),
          this, SLOT(change_short_description_requested()));

  connect(&model, SIGNAL(long_description_changed(QString)),
          this, SLOT(update_long_description_field()));
  connect(ui.long_description_field, SIGNAL(editing_finished()),
          this, SLOT(change_long_description_requested()));

  connect(&model, SIGNAL(author_changed(QString)),
          this, SLOT(update_author_field()));
  connect(ui.author_field, SIGNAL(editingFinished()),
          this, SLOT(change_author_requested()));

  connect(&model, SIGNAL(quest_version_changed(QString)),
          this, SLOT(update_quest_version_field()));
  connect(ui.quest_version_field, SIGNAL(editingFinished()),
          this, SLOT(change_quest_version_requested()));

  connect(&model, SIGNAL(release_date_changed(QDate)),
          this, SLOT(update_release_date_field()));
  connect(ui.release_status_progress_radio, SIGNAL(clicked()),
          this, SLOT(change_release_date_requested()));
  connect(ui.release_status_released_radio, SIGNAL(clicked()),
          this, SLOT(change_release_date_requested()));
  connect(ui.release_date_field, SIGNAL(dateChanged(QDate)),
          this, SLOT(change_release_date_requested()));

  connect(&model, SIGNAL(website_changed(QString)),
          this, SLOT(update_website_field()));
  connect(ui.website_field, SIGNAL(editingFinished()),
          this, SLOT(change_website_requested()));

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

  model.save();
}

/**
 * @brief Update field values.
 */
void QuestPropertiesEditor::update() {

  ui.solarus_version_value->setText(model.get_solarus_version());

  update_write_dir_field();
  update_title_field();
  update_short_description_field();
  update_long_description_field();
  update_author_field();
  update_quest_version_field();
  update_release_date_field();
  update_website_field();
  update_normal_size_field();
  update_min_size_field();
  update_max_size_field();
}

/**
 * @brief Update the write directory field.
 */
void QuestPropertiesEditor::update_write_dir_field() {

  ui.write_dir_field->setText(model.get_write_dir());
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
 * @brief Updates the title field.
 */
void QuestPropertiesEditor::update_title_field() {

  ui.title_field->setText(model.get_title());
}

/**
 * @brief Slot called when the user changes the title.
 */
void QuestPropertiesEditor::change_title_requested() {

  QString title = ui.title_field->text();
  QString old_title = model.get_title();
  if (title == old_title) {
    // No change.
    return;
  }

  try_command(new SetTitleCommand(*this, title));
}

/**
 * @brief Updates the short description field.
 */
void QuestPropertiesEditor::update_short_description_field() {

  ui.short_description_field->setText(model.get_short_description());
}

/**
 * @brief Slot called when the user changes the short description.
 */
void QuestPropertiesEditor::change_short_description_requested() {

  QString short_description = ui.short_description_field->text();
  QString old_short_description = model.get_short_description();
  if (short_description == old_short_description) {
    // No change.
    return;
  }

  try_command(new SetShortDescriptionCommand(*this, short_description));
}

/**
 * @brief Updates the long description field.
 */
void QuestPropertiesEditor::update_long_description_field() {

  ui.long_description_field->setPlainText(model.get_long_description());
}

/**
 * @brief Slot called when the user changes the long description.
 */
void QuestPropertiesEditor::change_long_description_requested() {

  QString long_description = ui.long_description_field->toPlainText();
  QString old_long_description = model.get_long_description();
  if (long_description == old_long_description) {
    // No change.
    return;
  }

  try_command(new SetLongDescriptionCommand(*this, long_description));
}

/**
 * @brief Updates the author field.
 */
void QuestPropertiesEditor::update_author_field() {

  ui.author_field->setText(model.get_author());
}

/**
 * @brief Slot called when the user changes the author.
 */
void QuestPropertiesEditor::change_author_requested() {

  QString author = ui.author_field->text();
  QString old_author = model.get_author();
  if (author == old_author) {
    // No change.
    return;
  }

  try_command(new SetAuthorCommand(*this, author));
}

/**
 * @brief Updates the quest version field.
 */
void QuestPropertiesEditor::update_quest_version_field() {

  ui.quest_version_field->setText(model.get_quest_version());
}

/**
 * @brief Slot called when the user changes the quest version.
 */
void QuestPropertiesEditor::change_quest_version_requested() {

  QString quest_version = ui.quest_version_field->text();
  QString old_quest_version = model.get_quest_version();
  if (quest_version == old_quest_version) {
    // No change.
    return;
  }

  try_command(new SetQuestVersionCommand(*this, quest_version));
}

/**
 * @brief Updates the short release date field.
 */
void QuestPropertiesEditor::update_release_date_field() {

  QDate release_date = model.get_release_date();
  if (!release_date.isValid()) {
    ui.release_status_progress_radio->setChecked(true);
    ui.release_date_field->setEnabled(false);
  }
  else {
    ui.release_status_released_radio->setChecked(true);
    ui.release_date_field->setEnabled(true);
    ui.release_date_field->setDate(release_date);
  }
}

/**
 * @brief Slot called when the user changes the release date.
 */
void QuestPropertiesEditor::change_release_date_requested() {

  QDate release_date;
  if (ui.release_status_released_radio->isChecked()) {
    release_date = ui.release_date_field->date();
  }
  QDate old_release_date = model.get_release_date();
  if (release_date == old_release_date) {
    return;
  }

  try_command(new SetReleaseDateCommand(*this, release_date));
}

/**
 * @brief Updates the website field.
 */
void QuestPropertiesEditor::update_website_field() {

  ui.website_field->setText(model.get_website());
}

/**
 * @brief Slot called when the user changes the website.
 */
void QuestPropertiesEditor::change_website_requested() {

  QString website = ui.website_field->text();
  QString old_website = model.get_website();
  if (website == old_website) {
    // No change.
    return;
  }

  try_command(new SetWebsiteCommand(*this, website));
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

}
