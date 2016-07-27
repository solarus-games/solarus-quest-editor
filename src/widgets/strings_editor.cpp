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
#include "widgets/gui_tools.h"
#include "widgets/strings_editor.h"
#include "widgets/new_string_dialog.h"
#include "widgets/change_string_key_dialog.h"
#include "editor_exception.h"
#include "quest.h"
#include "strings_model.h"
#include <QUndoStack>
#include <QMessageBox>
#include <cmath>

namespace SolarusEditor {

namespace {

/**
 * @brief Parent class of all undoable commands of the strings editor.
 */
class StringsEditorCommand : public QUndoCommand {

public:

  StringsEditorCommand(StringsEditor& editor, const QString& text) :
    QUndoCommand(text),
    editor(editor) {
  }

  StringsEditor& get_editor() const {
    return editor;
  }

  StringsModel& get_model() const {
    return editor.get_model();
  }

private:

  StringsEditor& editor;

};

/**
 * @brief Create a string.
 */
class CreateStringCommand : public StringsEditorCommand {

public:

  CreateStringCommand(
      StringsEditor& editor, const QString& key, const QString& value = "") :
    StringsEditorCommand(editor, StringsEditor::tr("Create string")),
    key(key),
    value(value) {
  }

  virtual void undo() override {

    get_model().delete_string(key);
  }

  virtual void redo() override {

    get_model().create_string(key, value);
    get_model().set_selected_key(key);
  }

private:

  QString key;
  QString value;
};

/**
 * @brief Duplicate string(s).
 */
class DuplicateStringsCommand : public StringsEditorCommand {

public:

  DuplicateStringsCommand(
      StringsEditor& editor, const QString& prefix, const QString& new_prefix) :
    StringsEditorCommand(editor, StringsEditor::tr("Duplicate strings")),
    prefix(prefix),
    new_prefix(new_prefix) {
  }

  virtual void undo() override {

    get_model().delete_prefix(new_prefix);
  }

  virtual void redo() override {

    get_model().duplicate_strings(prefix, new_prefix);
    get_model().set_selected_key(new_prefix);
  }

private:

  QString prefix;
  QString new_prefix;
};

/**
 * @brief Change string key.
 */
class SetStringKeyCommand : public StringsEditorCommand {

public:

  SetStringKeyCommand(
      StringsEditor& editor, const QString& key, const QString& new_key) :
    StringsEditorCommand(editor, StringsEditor::tr("Change string key")),
    old_key(key),
    new_key(new_key) {
  }

  virtual void undo() override {

    get_model().set_string_key(new_key, old_key);
    get_model().set_selected_key(old_key);
  }

  virtual void redo() override {

    get_model().set_string_key(old_key, new_key);
    get_model().set_selected_key(new_key);
  }

private:

  QString old_key;
  QString new_key;
};

/**
 * @brief Change string key.
 */
class SetKeyPrefixCommand : public StringsEditorCommand {

public:

  SetKeyPrefixCommand(
      StringsEditor& editor, const QString& olf_prefix,
      const QString& new_prefix) :
    StringsEditorCommand(editor, StringsEditor::tr("Change string key prefix")),
    old_prefix(olf_prefix),
    new_prefix(new_prefix) {
  }

  virtual void undo() override {

    Q_FOREACH (const auto& pair, edited_keys) {
      get_model().set_string_key(pair.second, pair.first);
    }
    if (!edited_keys.isEmpty()) {
      get_model().set_selected_key(edited_keys.front().first);
    }
  }

  virtual void redo() override {

    edited_keys = get_model().set_string_key_prefix(old_prefix, new_prefix);
    if (!edited_keys.isEmpty()) {
      get_model().set_selected_key(edited_keys.front().second);
    }
  }

private:

  QString old_prefix;
  QString new_prefix;
  QList<QPair<QString, QString>> edited_keys;
};

/**
 * @brief Delete a string.
 */
class DeleteStringCommand : public StringsEditorCommand {

public:

  DeleteStringCommand(StringsEditor& editor, const QString& key) :
    StringsEditorCommand(editor, StringsEditor::tr("Delete string")),
    key(key),
    value(get_model().get_string(key)) {
  }

  virtual void undo() override {

    get_model().create_string(key, value);
    get_model().set_selected_key(key);
  }

  virtual void redo() override {

    get_model().delete_string(key);
  }

private:

  QString key;
  QString value;
};

/**
 * @brief Delete several strings.
 */
class DeleteStringsCommand : public StringsEditorCommand {

public:

  DeleteStringsCommand(StringsEditor& editor, const QString& prefix) :
    StringsEditorCommand(editor, StringsEditor::tr("Delete strings")),
    prefix(prefix) {
  }

  virtual void undo() override {

    Q_FOREACH (const auto& pair, values) {
      get_model().create_string(pair.first, pair.second);
    }
    if (!values.isEmpty()) {
      get_model().set_selected_key(values.front().first);
    }
  }

  virtual void redo() override {

    values = get_model().delete_prefix(prefix);
  }

private:

  QString prefix;
  QList<QPair<QString, QString>> values;
};

/**
 * @brief Change string value.
 */
class SetStringValueCommand : public StringsEditorCommand {

public:

  SetStringValueCommand(
      StringsEditor& editor, const QString& key, const QString& value) :
    StringsEditorCommand(editor, StringsEditor::tr("Change string value")),
    key(key),
    old_value(get_model().get_string(key)),
    new_value(value) {
  }

  virtual void undo() override {

    get_model().set_string(key, old_value);
    get_model().set_selected_key(key);
  }

  virtual void redo() override {

    get_model().set_string(key, new_value);
    get_model().set_selected_key(key);
  }

private:

  QString key;
  QString old_value;
  QString new_value;
};

}

/**
 * @brief Creates a strings editor.
 * @param quest The quest containing the file.
 * @param language_id Language id of the strings data file to open.
 * @param parent The parent object or nullptr.
 * @throws EditorException If the file could not be opened.
 */
StringsEditor::StringsEditor(
    Quest& quest, const QString& language_id, QWidget* parent) :
  Editor(quest, quest.get_strings_path(language_id), parent),
  language_id(language_id),
  model(nullptr),
  quest(quest) {

  ui.setupUi(this);

  // Open the file.
  model = new StringsModel(quest, language_id, this);
  get_undo_stack().setClean();

  // Editor properties.
  set_title(tr("Strings %1").arg(language_id));
  set_icon(QIcon(":/images/icon_resource_language.png"));
  set_close_confirm_message(
        tr("Strings '%1' has been modified. Save changes?").arg(language_id));

  // Prepare the gui.
  ui.strings_tree_view->set_model(model);
  ui.strings_tree_view->setColumnWidth(0, 300);
  ui.strings_tree_view->setColumnHidden(2, true);

  ui.translation_field->set_resource_type(ResourceType::LANGUAGE);
  ui.translation_field->set_quest(quest);
  ui.translation_field->remove_id(language_id);
  ui.translation_field->add_special_value("", tr("<No language>"), 0);
  ui.translation_field->set_selected_id("");

  update();

  // Make connections.
  connect(&get_resources(),
          SIGNAL(element_description_changed(ResourceType, const QString&, const QString&)),
          this, SLOT(update_description_to_gui()));
  connect(ui.description_field, SIGNAL(editingFinished()),
          this, SLOT(set_description_from_gui()));

  connect(ui.create_button, SIGNAL(clicked()),
          this, SLOT(create_string_requested()));
  connect(ui.strings_tree_view, SIGNAL(create_string_requested()),
          this, SLOT(create_string_requested()));

  connect(ui.duplicate_button, SIGNAL(clicked()),
          this, SLOT(duplicate_string_requested()));
  connect(ui.strings_tree_view, SIGNAL(duplicate_string_requested()),
          this, SLOT(duplicate_string_requested()));

  connect(ui.set_key_button, SIGNAL(clicked()),
          this, SLOT(change_string_key_requested()));
  connect(ui.strings_tree_view, SIGNAL(set_string_key_requested()),
          this, SLOT(change_string_key_requested()));

  connect(ui.delete_button, SIGNAL(clicked()),
          this, SLOT(delete_string_requested()));
  connect(ui.strings_tree_view, SIGNAL(delete_string_requested()),
          this, SLOT(delete_string_requested()));

  connect(&model->get_selection_model(),
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(update_selection()));

  connect(model, SIGNAL(set_value_requested(QString,QString)),
          this, SLOT(set_value_requested(QString,QString)));

  connect(ui.translation_field, SIGNAL(activated(QString)),
          this, SLOT(translation_selector_activated()));
  connect(ui.translation_refresh_button, SIGNAL(clicked()),
          this, SLOT(translation_refresh_requested()));
}

/**
 * @brief Destructor.
 */
StringsEditor::~StringsEditor() {
  if (model != nullptr) {
    delete model;
  }
}

/**
 * @brief Returns the strings model being edited.
 * @return The strings model.
 */
StringsModel& StringsEditor::get_model() {
  return *model;
}

/**
 * @copydoc Editor::save
 */
void StringsEditor::save() {

  model->save();
}

/**
 * @brief Updates everything in the gui.
 */
void StringsEditor::update() {

  update_language_id_field();
  update_description_to_gui();
  update_selection();
}

/**
 * @brief Updates the language id displaying.
 */
void StringsEditor::update_language_id_field() {

  ui.language_id_field->setText(language_id);
}

/**
 * @brief Updates the content of the language description text edit.
 */
void StringsEditor::update_description_to_gui() {

  QString description = get_resources().get_description(
        ResourceType::LANGUAGE, language_id);
  if (ui.description_field->text() != description) {
    ui.description_field->setText(description);
  }
}

/**
 * @brief Modifies the language description in the quest resource list with
 * the new text entered by the user.
 *
 * If the new description is invalid, an error dialog is shown.
 */
void StringsEditor::set_description_from_gui() {

  QString description = ui.description_field->text();
  if (description == get_resources().get_description(
        ResourceType::LANGUAGE, language_id)) {
    return;
  }

  if (description.isEmpty()) {
    GuiTools::error_dialog(tr("Invalid description"));
    update_description_to_gui();
    return;
  }

  const bool was_blocked = blockSignals(true);
  try {
    get_resources().set_description(
          ResourceType::LANGUAGE, language_id, description);
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
void StringsEditor::update_selection() {

  // Ensures that the selected item is visible in the tree view
  QString key = model->get_selected_key();
  if (!key.isEmpty()) {
    ui.strings_tree_view->scrollTo(model->key_to_index(key));
  }

  // Update buttons
  bool enable = !key.isEmpty() && model->prefix_exists(key);
  ui.set_key_button->setEnabled(enable);
  ui.duplicate_button->setEnabled(enable);
  ui.delete_button->setEnabled(enable);
}

/**
 * @brief Slot called when the user wants to create a string.
 */
void StringsEditor::create_string_requested() {

  NewStringDialog dialog(model, model->get_selected_key(), "", this);

  int result = dialog.exec();
  if (result != QDialog::Accepted) {
    return;
  }

  QString key = dialog.get_string_key();
  QString value = dialog.get_string_value();
  try_command(new CreateStringCommand(*this, key, value));
}

/**
 * @brief Slot called when the user wants to duplicate string(s).
 */
void StringsEditor::duplicate_string_requested() {

  QString prefix = model->get_selected_key();
  QString new_prefix = prefix + tr("_copy");
  QString key;
  if (!model->can_duplicate_strings(prefix, new_prefix, key)) {
    GuiTools::error_dialog(tr("String '%1' already exists").arg(key));
    return;
  }

  try_command(new DuplicateStringsCommand(*this, prefix, new_prefix));
}

/**
 * @brief Slot called when the user wants to change the key of a string.
 */
void StringsEditor::change_string_key_requested() {

  if (model->is_selection_empty()) {
    return;
  }

  QString old_key = model->get_selected_key();
  QStringList prefix_keys = model->get_keys(old_key);
  bool exists = false;
  bool is_prefix = false;

  if (prefix_keys.size() > 0) {
    if (model->string_exists(old_key)) {
      exists = true;
      is_prefix = prefix_keys.size() > 1;
    } else {
      is_prefix = true;
    }
  } else {
    return;
  }

  ChangeStringKeyDialog dialog(
        model, old_key, is_prefix, is_prefix && exists, this);

  int result = dialog.exec();
  if (result != QDialog::Accepted) {
    return;
  }

  QString new_key = dialog.get_string_key();

  if (new_key == old_key) {
    return;
  }

  if (dialog.get_prefix()) {
    try_command(new SetKeyPrefixCommand(*this, old_key, new_key));
  } else {
    try_command(new SetStringKeyCommand(*this, old_key, new_key));
  }
}

/**
 * @brief Slot called when the user wants to delete a string.
 */
void StringsEditor::delete_string_requested() {

  if (model->is_selection_empty()) {
    return;
  }

  QString key = model->get_selected_key();
  if (!model->prefix_exists(key)) {
    return;
  }

  if (model->string_exists(key)) {
    try_command(new DeleteStringCommand(*this, key));
    return;
  }

  QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        tr("Delete confirmation"),
        tr("Do you really want to delete all strings prefixed by '%1'?").arg(key),
        QMessageBox::Yes | QMessageBox::No);

  if (answer != QMessageBox::Yes) {
    return;
  }

  try_command(new DeleteStringsCommand(*this, key));
}

/**
 * @brief Slot called when the user wants to set the value of a string.
 * @param key The key of the string to edit.
 * @param value The value to set.
 */
void StringsEditor::set_value_requested(
    const QString& key, const QString& value) {

  // If no exists, try to create.
  if (!model->string_exists(key)) {
    if (!value.isEmpty()) {
      try_command(new CreateStringCommand(*this, key, value));
    }
  }
  // Else,
  else {
    // If value is empty, try to remove.
    if (value.isEmpty()) {
      try_command(new DeleteStringCommand(*this, key));
    }
    // Else if the value is different, try to change.
    else if (value != model->get_string(key)) {
      try_command(new SetStringValueCommand(*this, key, value));
    }
  }
}

/**
 * @brief Slot called when the user changes the language in the selector.
 */
void StringsEditor::translation_selector_activated() {

  const QString& old_language_id = model->get_translation_id();
  const QString& new_language_id = ui.translation_field->get_selected_id();
  if (new_language_id == old_language_id) {
    // No change.
    return;
  }

  // If language id is empty.
  if (new_language_id.isEmpty()) {
    // Clear the translation and hide his column, return.
    model->clear_translation();
    ui.strings_tree_view->setColumnHidden(2, true);
    ui.translation_refresh_button->setEnabled(false);
    return;
  }


  // If the translation column is hide.
  if (ui.strings_tree_view->isColumnHidden(2)) {
    // Show the column.
    ui.strings_tree_view->setColumnHidden(2, false);

    // Resize value and translation columns.
    int col_width = ui.strings_tree_view->columnWidth(0);
    int width = ui.strings_tree_view->viewport()->width() - col_width;
    col_width = std::floor(width / 2.0);
    ui.strings_tree_view->setColumnWidth(1, col_width);
    ui.strings_tree_view->setColumnWidth(2, col_width);
  }

  // Set the translation.
  model->set_translation_id(new_language_id);
  ui.translation_refresh_button->setEnabled(true);
}

/**
 * @brief Slot called when the user wants to refresh the translation language.
 */
void StringsEditor::translation_refresh_requested() {
  if (ui.translation_field->get_selected_id().isEmpty()) {
    return;
  }
  model->reload_translation();
}

}
