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
#include "gui/dialogs_editor.h"
#include "gui/change_dialog_id_dialog.h"
#include "editor_exception.h"
#include "quest.h"
#include "dialogs_model.h"
#include <QUndoStack>
#include <QMessageBox>
#include <QInputDialog>

namespace {

/**
 * @brief Parent class of all undoable commands of the dialogs editor.
 */
class DialogsEditorCommand : public QUndoCommand {

public:

  DialogsEditorCommand(DialogsEditor& editor, const QString& text) :
    QUndoCommand(text),
    editor(editor) {
  }

  DialogsEditor& get_editor() const {
    return editor;
  }

  DialogsModel& get_model() const {
    return editor.get_model();
  }

private:

  DialogsEditor& editor;

};

/**
 * @brief Create a dialog.
 */
class CreateDialogCommand : public DialogsEditorCommand {

public:

  CreateDialogCommand(
      DialogsEditor& editor, const QString& id, const QString& text,
      const QMap<QString, QString>& properties = QMap<QString, QString>()) :
    DialogsEditorCommand(editor, DialogsEditor::tr("Create dialog")),
    id(id),
    text(text),
    properties(properties) {
  }

  virtual void undo() override {

    get_model().delete_dialog(id);
  }

  virtual void redo() override {

    get_model().create_dialog(id, text, properties);
    get_model().set_selected_id(id);
  }

private:

  QString id;
  QString text;
  QMap<QString, QString> properties;
};

/**
 * @brief Change dialog id.
 */
class SetDialogIdCommand : public DialogsEditorCommand {

public:

  SetDialogIdCommand(
      DialogsEditor& editor, const QString& id, const QString& new_id) :
    DialogsEditorCommand(editor, DialogsEditor::tr("Change dialog id")),
    old_id(id),
    new_id(new_id) {
  }

  virtual void undo() override {

    get_model().set_dialog_id(new_id, old_id);
    get_model().set_selected_id(old_id);
  }

  virtual void redo() override {

    get_model().set_dialog_id(old_id, new_id);
    get_model().set_selected_id(new_id);
  }

private:

  QString old_id;
  QString new_id;
};

/**
 * @brief Change dialog id.
 */
class SetIdPrefixCommand : public DialogsEditorCommand {

public:

  SetIdPrefixCommand(
      DialogsEditor& editor, const QString& old_prefix,
      const QString& new_prefix) :
    DialogsEditorCommand(editor, DialogsEditor::tr("Change dialog id prefix")),
    old_prefix(old_prefix),
    new_prefix(new_prefix) {
  }

  virtual void undo() override {

    for (auto pair : edited_ids) {
      get_model().set_dialog_id(pair.second, pair.first);
    }
    if (!edited_ids.isEmpty()) {
      get_model().set_selected_id(edited_ids.front().first);
    }
  }

  virtual void redo() override {

    edited_ids = get_model().set_dialog_id_prefix(old_prefix, new_prefix);
    if (!edited_ids.isEmpty()) {
      get_model().set_selected_id(edited_ids.front().second);
    }
  }

private:

  QString old_prefix;
  QString new_prefix;
  QList<QPair<QString, QString>> edited_ids;
};

/**
 * @brief Delete a dialog.
 */
class DeleteDialogCommand : public DialogsEditorCommand {

public:

  DeleteDialogCommand(DialogsEditor& editor, const QString& id) :
    DialogsEditorCommand(editor, DialogsEditor::tr("Delete dialog")),
    id(id),
    text(get_model().get_dialog_text(id)),
    properties(get_model().get_dialog_properties(id)) {
  }

  virtual void undo() override {

    get_model().create_dialog(id, text, properties);
    get_model().set_selected_id(id);
  }

  virtual void redo() override {

    get_model().delete_dialog(id);
  }

private:

  QString id;
  QString text;
  QMap<QString, QString> properties;
};

/**
 * @brief Delete several strings.
 */
class DeleteDialogsCommand : public DialogsEditorCommand {

public:

  DeleteDialogsCommand(DialogsEditor& editor, const QString& prefix) :
    DialogsEditorCommand(editor, DialogsEditor::tr("Delete dialogs")),
    prefix(prefix) {
  }

  virtual void undo() override {

    for (auto pair : removed_dialogs) {
      get_model().create_dialog(pair.first, pair.second);
    }
    if (!removed_dialogs.isEmpty()) {
      get_model().set_selected_id(removed_dialogs.front().first);
    }
  }

  virtual void redo() override {

    removed_dialogs = get_model().delete_prefix(prefix);
  }

private:

  QString prefix;
  QList<QPair<QString, Solarus::DialogData>> removed_dialogs;
};

/**
 * @brief Change dialog text.
 */
class SetDialogTextCommand : public DialogsEditorCommand {

public:

  SetDialogTextCommand(
      DialogsEditor& editor, const QString& id, const QString& text) :
    DialogsEditorCommand(editor, DialogsEditor::tr("Change dialog text")),
    id(id),
    old_text(get_model().get_dialog_text(id)),
    new_text(text) {
  }

  virtual void undo() override {

    get_model().set_dialog_text(id, old_text);
    get_model().set_selected_id(id);
  }

  virtual void redo() override {

    get_model().set_dialog_text(id, new_text);
    get_model().set_selected_id(id);
  }

private:

  QString id;
  QString old_text;
  QString new_text;
};

/**
 * @brief Create a dialog property.
 */
class CreateDialogPropertyCommand : public DialogsEditorCommand {

public:

  CreateDialogPropertyCommand(
      DialogsEditor& editor, const QString& id,
      const QString& key, const QString& value) :
    DialogsEditorCommand(editor, DialogsEditor::tr("Create dialog property")),
    id(id),
    key(key),
    value(value) {
  }

  virtual void undo() override {

    get_model().delete_dialog_property(id, key);
    get_editor().update_properties_buttons();
  }

  virtual void redo() override {

    get_model().set_dialog_property(id, key, value);
    get_editor().set_selected_property(key);
  }

private:

  QString id;
  QString key;
  QString value;
};

/**
 * @brief Delete a dialog property.
 */
class DeleteDialogPropertyCommand : public DialogsEditorCommand {

public:

  DeleteDialogPropertyCommand(
      DialogsEditor& editor, const QString& id, const QString& key) :
    DialogsEditorCommand(editor, DialogsEditor::tr("Delete dialog property")),
    id(id),
    key(key),
    value(get_model().get_dialog_property(id, key)) {
  }

  virtual void undo() override {

    get_model().set_dialog_property(id, key, value);
    get_editor().set_selected_property(key);
  }

  virtual void redo() override {

    get_model().delete_dialog_property(id, key);
    get_editor().update_properties_buttons();
  }

private:

  QString id;
  QString key;
  QString value;
};

/**
 * @brief Change a dialog property key.
 */
class SetDialogPropertyKeyCommand : public DialogsEditorCommand {

public:

  SetDialogPropertyKeyCommand(
      DialogsEditor& editor, const QString& id,
      const QString& old_key, const QString& new_key) :
    DialogsEditorCommand(editor, DialogsEditor::tr("Change dialog property key")),
    id(id),
    old_key(old_key),
    new_key(new_key),
    value(get_model().get_dialog_property(id, old_key)) {
  }

  virtual void undo() override {

    get_model().delete_dialog_property(id, new_key);
    get_model().set_dialog_property(id, old_key, value);
    get_editor().set_selected_property(old_key);
  }

  virtual void redo() override {

    get_model().delete_dialog_property(id, old_key);
    get_model().set_dialog_property(id, new_key, value);
    get_editor().set_selected_property(new_key);
  }

private:

  QString id;
  QString old_key;
  QString new_key;
  QString value;
};

/**
 * @brief Change a dialog property value.
 */
class SetDialogPropertyValueCommand : public DialogsEditorCommand {

public:

  SetDialogPropertyValueCommand(
      DialogsEditor& editor, const QString& id,
      const QString& key, const QString& new_value) :
    DialogsEditorCommand(editor, DialogsEditor::tr("Change dialog property")),
    id(id),
    key(key),
    old_value(get_model().get_dialog_property(id, key)),
    new_value(new_value) {
  }

  virtual void undo() override {

    get_model().set_dialog_property(id, key, old_value);
    get_editor().set_selected_property(key);
  }

  virtual void redo() override {

    get_model().set_dialog_property(id, key, new_value);
    get_editor().set_selected_property(key);
  }

private:

  QString id;
  QString key;
  QString old_value;
  QString new_value;
};

}

/**
 * @brief Creates a dialogs editor.
 * @param quest The quest containing the file.
 * @param language_id Language id of the strings data file to open.
 * @param parent The parent object or nullptr.
 * @throws EditorException If the file could not be opened.
 */
DialogsEditor::DialogsEditor(
    Quest& quest, const QString& language_id, QWidget* parent) :
  Editor(quest, quest.get_dialogs_path(language_id), parent),
  language_id(language_id),
  model(nullptr),
  quest(quest) {

  ui.setupUi(this);

  // Open the file.
  model = new DialogsModel(quest, language_id, this);
  get_undo_stack().setClean();

  // Editor properties.
  set_title(tr("Dialogs %1").arg(language_id));
  set_icon(QIcon(":/images/icon_resource_language.png"));
  set_close_confirm_message(
        tr("Dialogs '%1' has been modified. Save changes?").arg(language_id));

  // Prepare the gui.
  ui.dialogs_tree_view->set_model(model);
  ui.dialog_properties_table->set_model(model);

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
          this, SLOT(create_dialog_requested()));
  connect(ui.dialogs_tree_view, SIGNAL(create_dialog_requested()),
          this, SLOT(create_dialog_requested()));

  connect(ui.set_id_button, SIGNAL(clicked()),
          this, SLOT(change_dialog_id_requested()));
  connect(ui.dialogs_tree_view, SIGNAL(set_dialog_id_requested()),
          this, SLOT(change_dialog_id_requested()));

  connect(ui.delete_button, SIGNAL(clicked()),
          this, SLOT(delete_dialog_requested()));
  connect(ui.dialogs_tree_view, SIGNAL(delete_dialog_requested()),
          this, SLOT(delete_dialog_requested()));

  connect(&model->get_selection_model(),
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(update_selection()));
  connect(ui.dialog_properties_table, SIGNAL(itemSelectionChanged()),
          this, SLOT(update_properties_buttons()));

  connect(model, SIGNAL(dialog_id_changed(QString,QString)),
          this, SLOT(update_dialog_id_field()));

  connect(model, SIGNAL(dialog_text_changed(QString,QString)),
          this, SLOT(update_dialog_text_field()));
  connect(ui.dialog_text_field, SIGNAL(editing_finished()),
          this, SLOT(change_dialog_text_requested()));

  connect(ui.create_property_button, SIGNAL(clicked()),
          this, SLOT(create_dialog_property_requested()));
  connect(ui.dialog_properties_table, SIGNAL(create_property_requested()),
          this, SLOT(create_dialog_property_requested()));

  connect(ui.set_property_key_button, SIGNAL(clicked()),
          this, SLOT(change_dialog_property_key_requested()));
  connect(ui.dialog_properties_table, SIGNAL(set_property_key_requested()),
          this, SLOT(change_dialog_property_key_requested()));

  connect(ui.dialog_properties_table,
          SIGNAL(set_property_value_requested(QString,QString)),
          this, SLOT(change_dialog_property_value_requested(QString,QString)));

  connect(ui.delete_property_button, SIGNAL(clicked()),
          this, SLOT(delete_dialog_property_requested()));
  connect(ui.dialog_properties_table, SIGNAL(delete_property_requested()),
          this, SLOT(delete_dialog_property_requested()));

  connect(ui.dialog_properties_table, SIGNAL(set_from_translation_requested()),
          this, SLOT(set_dialog_property_from_translation_requested()));

  connect(ui.translation_field, SIGNAL(activated(QString)),
          this, SLOT(translation_selector_activated()));
  connect(ui.translation_refresh_button, SIGNAL(clicked()),
          this, SLOT(translation_refresh_requested()));

  connect(ui.display_margin_check_box, SIGNAL(clicked()),
          this, SLOT(update_display_margin()));
  connect(ui.display_margin_field, SIGNAL(editingFinished()),
          this, SLOT(update_display_margin()));
}

/**
 * @brief Destructor.
 */
DialogsEditor::~DialogsEditor() {
  if (model != nullptr) {
    delete model;
  }
}

/**
 * @brief Returns the dialogs model being edited.
 * @return The dialogs model.
 */
DialogsModel& DialogsEditor::get_model() {
  return *model;
}

/**
 * @brief Changes the current selected property.
 * @param key The key of the property to select.
 */
void DialogsEditor::set_selected_property(const QString& key) {

  ui.dialog_properties_table->set_selected_property(key);
  update_properties_buttons();
}

/**
 * @copydoc Editor::save
 */
void DialogsEditor::save() {

  model->save();
}

/**
 * @brief Updates everything in the gui.
 */
void DialogsEditor::update() {

  update_language_id_field();
  update_description_to_gui();
  update_selection();
}

/**
 * @brief Updates the language id displaying.
 */
void DialogsEditor::update_language_id_field() {

  ui.language_id_field->setText(language_id);
}

/**
 * @brief Updates the content of the language description text edit.
 */
void DialogsEditor::update_description_to_gui() {

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
void DialogsEditor::set_description_from_gui() {

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

  QSignalBlocker(this);
  try {
    get_resources().set_description(
          ResourceType::LANGUAGE, language_id, description);
    get_resources().save();
  }
  catch (const EditorException& ex) {
    ex.print_message();
  }
}

/**
 * @brief Updates the selection.
 */
void DialogsEditor::update_selection() {

  update_dialog_view();

  // Ensures that the selected item is visible in the tree view
  QString id = model->get_selected_id();
  if (!id.isEmpty()) {
    ui.dialogs_tree_view->scrollTo(model->id_to_index(id));
  }

  // Update buttons
  bool enable = !id.isEmpty() && model->prefix_exists(id);
  ui.set_id_button->setEnabled(enable);
  ui.delete_button->setEnabled(enable);
}

/**
 * @brief Slot called when the user wants to create a dialog.
 */
void DialogsEditor::create_dialog_requested() {

  bool ok;
  QString id = QInputDialog::getText(
        this, tr("New dialog"), tr("New dialog id:"),
        QLineEdit::Normal, model->get_selected_id(), &ok);

  if (!ok) {
    return;
  }

  if (!model->is_valid_id(id)) {
    GuiTools::error_dialog(tr("Invalid dialog id: %1").arg(id));
    return;
  }

  if (model->dialog_exists(id)) {
    GuiTools::error_dialog(tr("Dialog '%1' already exists").arg(id));
    return;
  }

  try_command(new CreateDialogCommand(*this, id, ""));
}

/**
 * @brief Slot called when the user wants to change the id of a dialog.
 */
void DialogsEditor::change_dialog_id_requested() {

  if (model->is_selection_empty()) {
    return;
  }

  QString old_id = model->get_selected_id();
  QStringList prefix_ids = model->get_ids(old_id);
  bool exists = false;
  bool is_prefix = false;

  if (prefix_ids.size() > 0) {
    if (model->dialog_exists(old_id)) {
      exists = true;
      is_prefix = prefix_ids.size() > 1;
    } else {
      is_prefix = true;
    }
  } else {
    return;
  }

  ChangeDialogIdDialog dialog(
        model, old_id, is_prefix, is_prefix && exists, this);

  int result = dialog.exec();
  if (result != QDialog::Accepted) {
    return;
  }

  QString new_id = dialog.get_dialog_id();

  if (new_id == old_id) {
    return;
  }

  if (dialog.get_prefix()) {
    try_command(new SetIdPrefixCommand(*this, old_id, new_id));
  } else {
    try_command(new SetDialogIdCommand(*this, old_id, new_id));
  }
}

/**
 * @brief Slot called when the user wants to delete a dialog.
 */
void DialogsEditor::delete_dialog_requested() {

  if (model->is_selection_empty()) {
    return;
  }

  QString id = model->get_selected_id();
  if (!model->prefix_exists(id)) {
    return;
  }

  if (model->dialog_exists(id)) {
    try_command(new DeleteDialogCommand(*this, id));
    return;
  }

  QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        tr("Delete confirmation"),
        tr("Do you really want to delete all dialogs prefixed by '%1'?").arg(id),
        QMessageBox::Yes | QMessageBox::No);

  if (answer != QMessageBox::Yes) {
    return;
  }

  try_command(new DeleteDialogsCommand(*this, id));
}

/**
 * @brief Updates the dialog view.
 */
void DialogsEditor::update_dialog_view() {

  // Ensures that the selected item is visible in the tree view
  QString id = model->get_selected_id();
  bool exists = model->dialog_exists(id);
  bool has_translation = model->translated_dialog_exists(id);

  ui.dialog_properties_group_box->setEnabled(exists || has_translation);

  update_dialog_id_field();
  update_dialog_text_field();
  update_translation_text_field();
  update_properties_buttons();
}

/**
 * @brief Updates the dialog id field.
 */
void DialogsEditor::update_dialog_id_field() {

  QString id = model->get_selected_id();

  if (model->dialog_exists(id) || model->translated_dialog_exists(id)) {
    ui.dialog_id_field->setText(id);
  } else {
    ui.dialog_id_field->setText("");
  }
}

/**
 * @brief Updates the dialog text field.
 */
void DialogsEditor::update_dialog_text_field() {

  QString id = model->get_selected_id();

  if (model->dialog_exists(id)) {
    ui.dialog_text_field->setPlainText(model->get_dialog_text(id));
  } else {
    ui.dialog_text_field->setPlainText("");
  }
}

/**
 * @brief Slot called when the user wants to change the id of a dialog.
 */
void DialogsEditor::change_dialog_text_requested() {

  QString id = model->get_selected_id();
  QString new_text = ui.dialog_text_field->toPlainText();

  if (!model->dialog_exists(id)) {
    if (!new_text.isEmpty()) {
      try_command(new CreateDialogCommand(*this, id, new_text));
    }
    return;
  }

  QString old_text = model->get_dialog_text(id);

  if (new_text == old_text) {
    return;
  }

  try_command(new SetDialogTextCommand(*this, id, new_text));
}

/**
 * @brief Updates the translation text field.
 */
void DialogsEditor::update_translation_text_field() {

  QString id = model->get_selected_id();

  if (model->translated_dialog_exists(id)) {
    ui.translation_text_field ->setPlainText(
          model->get_translated_dialog_text(id));
    ui.translation_text_field->setEnabled(true);
  } else {
    ui.translation_text_field->setPlainText("");
    ui.translation_text_field->setEnabled(false);
  }
}

/**
 * @brief Updates the properties buttons.
 */
void DialogsEditor::update_properties_buttons() {

  QString id = model->get_selected_id();
  QString key = ui.dialog_properties_table->get_selected_property();

  bool enable = model->dialog_property_exists(id, key);
  ui.set_property_key_button->setEnabled(enable);
  ui.delete_property_button->setEnabled(enable);
}

/**
 * @brief Slot called when the user wants to create a new dialog property.
 */
void DialogsEditor::create_dialog_property_requested() {

  bool ok;
  QString key = QInputDialog::getText(
        this, tr("New dialog property"),
        tr("New property key:"), QLineEdit::Normal,
        ui.dialog_properties_table->get_selected_property(), &ok);

  if (!ok || key.isEmpty()) {
    return;
  }

  QString id = model->get_selected_id();
  if (model->dialog_property_exists(id, key)) {
    GuiTools::error_dialog(
      tr("The property '%1' already exists in the dialog '%2'").arg(key, id));
    return;
  }

  if (!model->dialog_exists(id)) {
    QMap<QString, QString> properties;
    properties.insert(key, "");
    try_command(new CreateDialogCommand(*this, id, "", properties));

  } else {
    try_command(new CreateDialogPropertyCommand(*this, id, key, ""));
  }
}

/**
 * @brief Slot called when the user wants to delete a dialog property.
 */
void DialogsEditor::delete_dialog_property_requested() {

  QString id = model->get_selected_id();
  QString key = ui.dialog_properties_table->get_selected_property();

  if (!model->dialog_property_exists(id, key)) {
    return;
  }

  try_command(new DeleteDialogPropertyCommand(*this, id, key));
}

/**
 * @brief Slot called when the user wants to change the key of a dialog property.
 */
void DialogsEditor::change_dialog_property_key_requested() {

  QString id = model->get_selected_id();
  QString old_key = ui.dialog_properties_table->get_selected_property();

  if (old_key.isEmpty() || !model->dialog_property_exists(id, old_key)) {
    return;
  }

  bool ok;
  QString new_key = QInputDialog::getText(
        this, tr("Change dialog property key"),
        tr("Change the key of the property '%1':").arg(old_key),
        QLineEdit::Normal, old_key, &ok);

  if (!ok || new_key == old_key) {
    return;
  }

  if (new_key.isEmpty()) {
    GuiTools::error_dialog(tr("The property key cannot be empty"));
    return;
  }

  try_command(new SetDialogPropertyKeyCommand(*this, id, old_key, new_key));
}

/**
 * @brief Slot called when the user change the value of a dialog property.
 */
void DialogsEditor::change_dialog_property_value_requested(
    const QString& key, const QString& value) {

  QString id = model->get_selected_id();

  if (!model->dialog_exists(id)) {
    QMap<QString, QString> properties;
    properties.insert(key, value);
    try_command(new CreateDialogCommand(*this, id, "", properties));

  } else if (!model->dialog_property_exists(id, key)) {
    try_command(new CreateDialogPropertyCommand(*this, id, key, value));

  } else if (value != model->get_dialog_property(id, key)) {
    try_command(new SetDialogPropertyValueCommand(*this, id, key, value));
  }
}

/**
 * @brief Slot called when the user wants to copy a dialog property from the translation.
 */
void DialogsEditor::set_dialog_property_from_translation_requested() {

  QString id = model->get_selected_id();
  QString key = ui.dialog_properties_table->get_selected_property();
  if (model->dialog_property_exists(id, key) ||
      !model->translated_dialog_exists(id)) {
    return;
  }

  QString value = model->get_translated_dialog_property(id, key);

  if (!model->dialog_exists(id)) {
    QMap<QString, QString> properties;
    properties.insert(key, value);
    try_command(new CreateDialogCommand(*this, id, "", properties));

  } else {
    try_command(new CreateDialogPropertyCommand(*this, id, key, value));
  }
}

/**
 * @brief Slot called when the user changes the language in the selector.
 */
void DialogsEditor::translation_selector_activated() {

  const QString& old_language_id = model->get_translation_id();
  const QString& new_language_id = ui.translation_field->get_selected_id();
  if (new_language_id == old_language_id) {
    // No change.
    return;
  }

  // If language id is empty.
  if (new_language_id.isEmpty()) {
    // Clear the translation, return.
    model->clear_translation();
    ui.translation_refresh_button->setEnabled(false);
  }
  else {
    // Set the translation.
    model->set_translation_id(new_language_id);
    ui.translation_refresh_button->setEnabled(true);
  }

  update_translation_text_field();
  ui.dialog_properties_table->update();
}

/**
 * @brief Slot called when the user wants to refresh the translation language.
 */
void DialogsEditor::translation_refresh_requested() {
  if (ui.translation_field->get_selected_id().isEmpty()) {
    return;
  }

  model->reload_translation();

  update_translation_text_field();
  ui.dialog_properties_table->update();
}

/**
 * @brief Slot called when the user changes the displayed margin in text edit.
 */
void DialogsEditor::update_display_margin() {

  bool display_margin = ui.display_margin_check_box->isChecked();
  int margin = ui.display_margin_field->value();

  ui.display_margin_field->setEnabled(display_margin);
  ui.dialog_text_field->set_show_margin(display_margin, margin);
  ui.translation_text_field->set_show_margin(display_margin, margin);
}
