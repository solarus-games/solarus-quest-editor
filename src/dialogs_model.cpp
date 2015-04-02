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
#include "editor_exception.h"
#include "quest.h"
#include "dialogs_model.h"
#include <QIcon>

using StringResources = Solarus::DialogResources;
using DialogData = Solarus::DialogData;

/**
 * @brief Creates a dialogs model.
 * @param quest The quest.
 * @param language_id Id of the language of dialogs to manage.
 * @param parent The parent object or nullptr.
 * @throws EditorException If the file could not be opened.
 */
DialogsModel::DialogsModel(
    const Quest& quest,
    const QString& language_id,
    QObject* parent) :
  QAbstractItemModel(parent),
  quest(quest),
  language_id(language_id),
  selection_model(this) {

  // Load the strings data file.
  QString path = quest.get_dialogs_path(language_id);
  if (!resources.import_from_file(path.toStdString())) {
    throw EditorException(tr("Cannot open dialogs data file '%1'").arg(path));
  }

  // Create the indexed tree.
  build_dialog_tree();
}

/**
 * @brief Destructor.
 */
DialogsModel::~DialogsModel() {
  dialog_tree.clear();
}

/**
 * @brief Returns the quest.
 */
const Quest& DialogsModel::get_quest() const {
  return quest;
}

/**
 * @brief Returns the language id of the dialogs managed by this model.
 * @return The language id.
 */
QString DialogsModel::get_language_id() const {
  return language_id;
}

/**
 * @brief Saves the dialogs to its data file.
 * @throws EditorException If the file could not be saved.
 */
void DialogsModel::save() const {

  QString path = quest.get_dialogs_path(language_id);

  if (!resources.export_to_file(path.toStdString())) {
    throw EditorException(tr("Cannot save dialogs data file '%1'").arg(path));
  }
}

/**
 * @brief Returns the number of columns in the indexed tree.
 * @param parent Parent index.
 * @return The number of columns.
 */
int DialogsModel::columnCount(const QModelIndex& /* parent */) const {
  return 1;
}

/**
 * @brief Returns the number of childs of a dialog id in the indexed tree.
 * @param parent Parent index.
 * @return The number of childs from parent.
 */
int DialogsModel::rowCount(const QModelIndex& parent) const {

  if (!parent.isValid()) {
    return dialog_tree.get_row_count();
  }

  if (parent.column() != 0) {
    return 0;
  }

  QString id = index_to_id(parent);
  if (!dialog_tree.key_exists(id)) {
    return 0;
  }
  return dialog_tree.get_row_count(id);
}

/**
 * @brief Returns the index of a dialog id in the indexed tree.
 * @param row Row of the string key.
 * @param column Column of the string key.
 * @param parent Parent of the string key.
 * @return The corresponding index. Returns an invalid index if there is no
 * such the dialog id.
 */
QModelIndex DialogsModel::index(
    int row, int column, const QModelIndex& parent) const {

  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  if (!parent.isValid()) {
    return id_to_index(dialog_tree.get_row_key(row));
  }

  QString id = index_to_id(parent);
  if (row >= dialog_tree.get_row_count(id)) {
    return QModelIndex();
  }

  return id_to_index(dialog_tree.get_row_key(row, id));
}

/**
 * @brief Returns the parent of the dialog id in the indexed tree.
 * @param index Index to get the parent of.
 * @return The parent index, or an invalid index if the dialog id has no parent.
 */
QModelIndex DialogsModel::parent(const QModelIndex& model_index) const {

  if (!model_index.isValid()) {
    return QModelIndex();
  }

  QString id = dialog_tree.get_parent(index_to_id(model_index));
  return id_to_index(id);
}

/**
 * @brief Returns whether an dialog id in the indexed tree has any children.
 * @param parent The dialog id to test.
 * @return @c true if this dialog id has children.
 */
bool DialogsModel::hasChildren(const QModelIndex& parent) const {

  return rowCount(parent) > 0;
}

/**
 * @brief Returns the data of a dialog id in the indexed tree for a given role.
 * @param index Index of the dialog id to get.
 * @param role The wanted role.
 * @return The data.
 */
QVariant DialogsModel::data(const QModelIndex& model_index, int role) const {

  if (model_index.column() != 0 || !model_index.isValid()) {
    return QVariant();
  }

  QString id = index_to_id(model_index);
  if (dialog_tree.key_exists(id)) {
    switch (role) {
      case Qt::DisplayRole: return id.split(".").back(); break;

      case Qt::DecorationRole:
        if (!dialog_exists(id)) {

          if (translated_dialog_exists(id)) {
            if (dialog_tree.get_row_count(id) == 0) {
              return QIcon(":/images/icon_dialog_missing.png");
            } else {
              return QIcon(":/images/icon_dialogs_missing.png");
            }
          }
          return QIcon(":/images/icon_folder_open.png");

        } else {
          if (dialog_tree.get_row_count(id) == 0) {
            return QIcon(":/images/icon_dialog.png");
          } else {
            return QIcon(":/images/icon_dialogs.png");
          }
        }
    }
  }

  return QVariant();
}

/**
 * @brief Returns whether an id is valid for dialogs.
 * @param id The id to test.
 * @return @c true if the id is valid.
 */
bool DialogsModel::is_valid_id(const QString& id) {
  return !id.isEmpty() && !id.startsWith(".") && !id.endsWith(".");
}

/**
 * @brief Returns the dialog id from specified model index.
 * @param index The model index.
 * @return The corresponding string key.
 */
QString DialogsModel::index_to_id(const QModelIndex& index) const {

  if (!index.isValid()) {
    return "";
  }
  return *static_cast<QString*>(index.internalPointer());
}

/**
 * @brief Returns the model index of a specific dialog id.
 * @param id The dialog id.
 * @return The correponding model index.
 */
QModelIndex DialogsModel::id_to_index(const QString& id) const {

  if (!dialog_tree.key_exists(id)) {
    return QModelIndex();
  }
  QString* internal_key = dialog_tree.get_internal_key(id);
  return createIndex(dialog_tree.get_index(id), 0, internal_key);
}

/**
 * @brief Returns whether there exists a dialog with the specified id.
 * @param id The id to test.
 * @return @c true if such the specified dialog exists.
 */
bool DialogsModel::dialog_exists(const QString& id) const {

  return resources.has_dialog(id.toStdString());
}

/**
 * @brief Returns whether there exists a dialog property with the specified id.
 * @param id The id to test.
 * @param key The key of the property.
 * @return @c true if such the specified property dialog exists.
 */
bool DialogsModel::dialog_property_exists(
    const QString& id, const QString& key) const {

  if (!dialog_exists(id)) {
    return false;
  }
  return resources.get_dialog(id.toStdString()).has_property(key.toStdString());
}

/**
 * @brief Returns whether exists id starts with a specified prefix.
 * @param prefix The prefix to test.
 * @return @c true if a key that starts with the specified prefix exists.
 */
bool DialogsModel::prefix_exists(const QString& prefix) const {

  for (const auto& kvp : resources.get_dialogs()) {
    if (QString::fromStdString(kvp.first).startsWith(prefix)) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Returns all the existing ids that start with the specified prefix.
 * @param prefix The prefix to test.
 * @return The list of all ids that starts with the specified prefix.
 */
QStringList DialogsModel::get_ids(const QString& prefix) const {

  QStringList list;
  for (const auto& kvp : resources.get_dialogs()) {
    QString id = QString::fromStdString(kvp.first);
    if (id.startsWith(prefix)) {
      list.push_back(id);
    }
  }
  return list;
}

/**
 * @brief Returns the data of a dialog with the specified id.
 * @param id The id to test.
 * @return the data of the specified dialog.
 */
DialogData DialogsModel::get_dialog_data(const QString& id) const {

  DialogData data;
  if (dialog_exists(id)) {
    data = resources.get_dialog(id.toStdString());
  }
  return data;
}

/**
 * @brief Returns the text of a dialog with the specified id.
 * @param id The id to test.
 * @return the text of the specified dialog or an empty string if no exists.
 */
QString DialogsModel::get_dialog_text(const QString& id) const {

  if (!dialog_exists(id)) {
    return "";
  }
  return QString::fromStdString(
        resources.get_dialog(id.toStdString()).get_text());
}

/**
 * @brief Returns the properties of a dialog with the specified id.
 * @param id The id to test.
 * @return the properties of the specified dialog.
 */
QMap<QString, QString> DialogsModel::get_dialog_properties(
    const QString& id) const {

  QMap<QString, QString> map;
  if (!dialog_exists(id)) {
    return map;
  }

  for (auto kvp : resources.get_dialog(id.toStdString()).get_properties()) {
    map.insert(
          QString::fromStdString(kvp.first),
          QString::fromStdString(kvp.second));
  }
  return map;
}

/**
 * @brief Returns a property of a dialog with the specified id.
 * @param id The id to test.
 * @param key The key of the property to test.
 * @return the property value or an empty string if no exists.
 */
QString DialogsModel::get_dialog_property(
    const QString& id, const QString& key) const {

  if (!dialog_property_exists(id, key)) {
    return "";
  }
  return QString::fromStdString(
        resources.get_dialog(id.toStdString()).get_property(key.toStdString()));
}

/**
 * @brief Creates a new dialog.
 *
 * The index of the selection may change, since dialog are sorted
 * alphabetically.
 * Emits rowsAboutToBeInserted(), adds the new dialog
 * and then emits rowsInserted(), as required by QAbstractItemModel.
 *
 * Then, emits dialog_created().
 *
 * The newly created dialog is not initially selected.
 * The existing selection is preserved, though the index of many
 * dialogs can change.
 * The selection is cleared before the operations and restored after,
 * updated with the new index.
 *
 * @param id Id of the dialog to create.
 * @param data Data of the dialog to create.
 * @throws EditorException in case of error.
 */
void DialogsModel::create_dialog(const QString& id, const DialogData& data) {

  // Make some checks first.
  if (!is_valid_id(id)) {
      throw EditorException(tr("Invalid dialog id: %1").arg(id));
  }

  if (dialog_exists(id)) {
      throw EditorException(tr("Dialog '%1' already exists").arg(id));
  }

  // Save and clear the selection since a lot of indexes may change.
  QString old_selection = get_selected_id();
  clear_selection();

  // Add to the strings file.
  resources.add_dialog(id.toStdString(), data);

  // Update the indexed tree.
  QString parent_id;
  int index;
  if (dialog_tree.add_key(id, parent_id, index)) {
    // Notify people before restoring the selection, so that they have a
    // chance to know new indexes before receiving selection signals.
    beginInsertRows(id_to_index(parent_id), index, index);
    endInsertRows();
  } else {
    QModelIndex model_index = id_to_index(id);
    dataChanged(model_index, model_index);
  }

  // Notify people.
  emit dialog_created(id);

  // Restore the selection.
  set_selected_id(old_selection);
}

/**
 * @brief Creates a new dialog.
 *
 * The index of the selection may change, since dialog are sorted
 * alphabetically.
 * Emits rowsAboutToBeInserted(), adds the new dialog
 * and then emits rowsInserted(), as required by QAbstractItemModel.
 *
 * Then, emits dialog_created().
 *
 * The newly created dialog is not initially selected.
 * The existing selection is preserved, though the index of many
 * dialogs can change.
 * The selection is cleared before the operations and restored after,
 * updated with the new index.
 *
 * @param id Id of the dialog to create.
 * @param text Text value of the dialog to create.
 * @param properties Properties of the dialog to create.
 * @throws EditorException in case of error.
 */
void DialogsModel::create_dialog(
    const QString &id, const QString &text,
    const QMap<QString, QString>& properties) {

  DialogData data;
  data.set_text(text.toStdString());
  for (QString key : properties.keys()) {
    data.set_property(key.toStdString(), properties.value(key).toStdString());
  }
  create_dialog(id, data);
}

/**
 * @brief Changes the text of the specified dialog.
 *
 * Emit dialog_text_changed() if there is a change.
 *
 * @param id The id of the dialog ot change.
 * @param text The new text value.
 */
void DialogsModel::set_dialog_text(const QString &id, const QString &text) {

  if (get_dialog_text(id) == text) {
    // No change.
    return;
  }
  resources.get_dialog(id.toStdString()).set_text(text.toStdString());

  // Notify people.
  emit dialog_text_changed(id, text);
}

/**
 * @brief Changes the property of the specified dialog.
 *
 * Emit dialog_text_changed() if there is a change.
 *
 * @param id The id of the dialog to change.
 * @param key The key of the property to change
 * @param value The new property value.
 */
void DialogsModel::set_dialog_property(
    const QString& id, const QString& key, const QString& value) {

  if (get_dialog_property(id, key) == value) {
    // No change.
    return;
  }
  bool exists = dialog_property_exists(id, key);

  resources.get_dialog(id.toStdString()).set_property(
        key.toStdString(), value.toStdString());

  // Notify people.
  if (exists) {
    emit dialog_property_changed(id, key, value);
  } else {
    emit dialog_property_created(id, key, value);
  }
}

/**
 * @brief Changes the id of a dialog.
 *
 * The index of multiple dialogs may change, since they are sorted alphabetically.
 * In this case, emits rowsAboutToBeRemoved(), removes the dialog,
 * emits rowsRemoved() and rowsAboutToBeInserted(), adds the new dialog
 * and then emits rowsInserted(), as required by QAbstractItemModel.
 *
 * Then, emits dialog_id_changed(), no matter if the index has also changed.
 *
 * The selection is preserved, though the index of many dialogs can change.
 * The selection is cleared before the operations and restored after,
 * updated with the new indexes.
 *
 * @param id Id of an existing dialog.
 * @param new_id The new id to set.
 * @return The new id of the dialog.
 * @throws EditorException in case of error.
 */
QString DialogsModel::set_dialog_id(const QString& id, const QString& new_id) {

  if (new_id == id) {
    // Nothing to do.
    return id;
  }

  // Make some checks first.
  if (!dialog_exists(id)) {
    throw EditorException(tr("Dialog '%1' no exists").arg(id));
  }

  if (dialog_exists(new_id)) {
    throw EditorException(tr("Dialog '%1' already exists").arg(new_id));
  }

  if (!is_valid_id(new_id)) {
    throw EditorException(tr("Invalid dialog id: %1").arg(new_id));
  }

  // Save and clear the selection since a lot of indexes may change.
  QString old_selection = get_selected_id();
  clear_selection();

  // Change in to the strings file.
  resources.set_dialog_id(id.toStdString(), new_id.toStdString());

  // Remove from the indexed tree.
  QString parent_id;
  int index;
  if (dialog_tree.can_remove_key(id, parent_id, index)) {

    // Call beginRemoveRows() as requested by QAbstractItemModel.
    beginRemoveRows(id_to_index(parent_id), index, index);
    dialog_tree.remove_key(id);
    endRemoveRows();
  } else if (dialog_tree.remove_key(id)) {
    QModelIndex model_index = id_to_index(id);
    dataChanged(model_index, model_index);
  }

  // Add to the indexed tree.
  if (dialog_tree.add_key(new_id, parent_id, index)) {

    // Call beginInsertRows() as requested by QAbstractItemModel.
    beginInsertRows(id_to_index(parent_id), index, index);
    endInsertRows();
  } else {
    QModelIndex model_index = id_to_index(id);
    dataChanged(model_index, model_index);
  }

  // Notify people.
  emit dialog_id_changed(id, new_id);

  // Restore the selection.
  set_selected_id(old_selection);
  return new_id;
}

/**
 * @brief Returns whether the prefix of dialog ids can be changed.
 * @param old_prefix[in] The prefix key of strings to change.
 * @param new_prefix[in] The new prefix to set.
 * @param id[out] The id that already exists in case of error.
 * @return @c true if the prefix can be changed.
 */
bool DialogsModel::can_set_dialog_id_prefix(
    const QString& old_prefix, const QString& new_prefix, QString& id) {

  for (QString prefixed_id : get_ids(old_prefix)) {

    prefixed_id.replace(QRegExp(QString("^") + old_prefix), new_prefix);
    if (dialog_exists(prefixed_id)) {
      id = prefixed_id;
      return false;
    }
  }
  return true;
}

/**
 * @brief Changes the prefix of dialog ids.
 * @param old_prefix The prefix key of strings to change.
 * @param new_prefix The new prefix to set.
 * @return The new ids of the dialogs.
 * @throws EditorException in case of error.
 */
QList<QPair<QString, QString>> DialogsModel::set_dialog_id_prefix(
    const QString& old_prefix, const QString& new_prefix) {

  // Check if the prefix can be changed.
  QString id;
  if (!can_set_dialog_id_prefix(old_prefix, new_prefix, id)) {
    throw EditorException(tr("Dialog '%1' already exists").arg(id));
  }

  // change the dialog ids.
  QList<QPair<QString, QString>> list;
  for (QString old_id : get_ids(old_prefix)) {

    QString new_id = old_id;
    new_id.replace(QRegExp(QString("^") + old_prefix), new_prefix);
    list.push_back(
      QPair<QString, QString>(old_id, set_dialog_id(old_id, new_id)));
  }
  return list;
}

/**
 * @brief Deletes a dialog.
 *
 * The index of multiple dialogs may change, since they are sorted alphabetically.
 * Emits rowsAboutToBeRemoved(), removes the dialog
 * and then emits rowsRemoved(), as required by QAbstractItemModel.
 *
 * Then, emits dialog_deleted().
 *
 * Except for the deleted dialog, the existing selection is preserved,
 * though the index of many dialogs can change.
 * The selection is cleared before the operations and restored after,
 * updated with the new indexes.
 *
 * @param id Id of the dialog to delete.
 * @throws EditorException in case of error.
 */
void DialogsModel::delete_dialog(const QString& id) {

  // Make some checks first.
  if (!dialog_exists(id)) {
    throw EditorException(tr("Invalid dialog id: %1").arg(id));
  }

  // Save and clear the selection since a lot of indexes may change.
  QString old_selection = get_selected_id();
  clear_selection();

  // Delete from the strings file
  resources.remove_dialog(id.toStdString());

  // Remove from the indexed tree.
  QString parent_id;
  int index;
  if (dialog_tree.can_remove_key(id, parent_id, index)) {

    // Call beginRemoveRows() as requested by QAbstractItemModel.
    beginRemoveRows(id_to_index(parent_id), index, index);

    dialog_tree.remove_key(id);

    // Notify people before restoring the selection, so that they have a
    // chance to know new indexes before receiving selection signals.
    endRemoveRows();
  } else if (dialog_tree.remove_key(id)) {
    QModelIndex model_index = id_to_index(id);
    dataChanged(model_index, model_index);
  }

  // Notify people.
  emit dialog_deleted(id);

  // Restore the selection.
  set_selected_id(old_selection);
}

/**
 * @brief Removes a property of the specified dialog.
 *
 * Emit dialog_property_deleted() if there is a remove.
 *
 * @param id The id of the dialog to remove.
 * @param key The key of the property to remove
 */
void DialogsModel::delete_dialog_property(
    const QString& id, const QString& key) {

  if (!dialog_property_exists(id, key)) {
    return;
  }

  resources.get_dialog(id.toStdString()).remove_property(key.toStdString());

  // Notify people.
  emit dialog_property_deleted(id, key);
}

/**
 * @brief Removes all dialogs with a specific prefix.
 * @param prefix The prefix id of dialogs to remove.
 * @return The list of all removed dialogs (key, value).
 * @throws EditorException in case of error.
 */
QList<QPair<QString, DialogData>> DialogsModel::delete_prefix(
    const QString& prefix) {

  QList<QPair<QString, DialogData>> list;
  for (QString key : get_ids(prefix)) {
    list.push_back(QPair<QString, DialogData>(key, get_dialog_data(key)));
    delete_dialog(key);
  }
  return list;
}

/**
 * @brief Returns the selection model.
 * @return The selection info.
 */
QItemSelectionModel& DialogsModel::get_selection_model() {
  return selection_model;
}

/**
 * @brief Returns whether no dialogs are selected.
 * @return @c true if the selection is empty.
 */
bool DialogsModel::is_selection_empty() const {
  return selection_model.selection().isEmpty();
}

/**
 * @brief Returns the id of the selected dialog.
 * @return The selected dialog id.
 * Returns an empty string if the selection is empty.
 */
QString DialogsModel::get_selected_id() const {

  QModelIndexList selected_indexes = selection_model.selectedIndexes();
  if (selected_indexes.isEmpty()) {
    return "";
  }
  return index_to_id(selected_indexes.first());
}

/**
 * @brief Selects a dialog and deselects all others.
 * @param id The id to select.
 */
void DialogsModel::set_selected_id(const QString& id) {

  if (!dialog_tree.key_exists(id)) {
    return clear_selection();
  }

  selection_model.select(id_to_index(id), QItemSelectionModel::ClearAndSelect);
}

/**
 * @brief Deselects all selected items.
 */
void DialogsModel::clear_selection() {
  selection_model.clear();
}

/**
 * @brief Returns the language id of the current translation.
 * @return The language if of the translation.
 */
QString DialogsModel::get_translation_id() const {
  return translation_id;
}

/**
 * @brief Changes the language of the current translation.
 * @param language_id The language id of the translation.
 */
void DialogsModel::set_translation_id(const QString& language_id) {

  if (language_id == translation_id) {
    return;
  }
  translation_id = language_id;
  reload_translation();
}

/**
 * @brief Removes the current translation.
 */
void DialogsModel::clear_translation() {

  if (translation_id.isEmpty()) {
    return;
  }

  translation_id = "";
  clear_translation_from_tree();
  translation_resources.clear();
}

/**
 * @brief Reload the current translation.
 */
void DialogsModel::reload_translation() {

  clear_translation_from_tree();

  QString path = quest.get_dialogs_path(translation_id);
  translation_resources.clear();
  if (!translation_resources.import_from_file(path.toStdString())) {
    translation_id = "";
    throw EditorException(tr("Cannot open dialogs data file '%1'").arg(path));
  }

  for (const auto& kvp : translation_resources.get_dialogs()) {
    QString id = QString::fromStdString(kvp.first);
    QString parent_id;
    int index;
    if (dialog_tree.add_ref(id, parent_id, index)) {
      beginInsertRows(id_to_index(parent_id), index, index);
      endInsertRows();
    }
  }
}

/**
 * @brief Returns whether a translated dialog exists.
 * @param id The id of the dialog.
 * @return @c true if the translated dialog exists.
 */
bool DialogsModel::translated_dialog_exists(const QString& id) const {

  return translation_resources.has_dialog(id.toStdString());
}

/**
 * @brief Returns a translated dialog text.
 * @param id The id of the dialog.
 * @return The translated dialog text or an empty string if no exists.
 */
QString DialogsModel::get_translated_dialog_text(const QString& id) const {

  if (!translated_dialog_exists(id)) {
    return "";
  }
  return QString::fromStdString(
        translation_resources.get_dialog(id.toStdString()).get_text());
}

/**
 * @brief Returns the properties of a dialog with the specified id.
 * @param id The id to test.
 * @return the properties of the specified dialog.
 */
QMap<QString, QString> DialogsModel::get_translated_dialog_properties(
    const QString& id) const {

  QMap<QString, QString> map;
  if (!translated_dialog_exists(id)) {
    return map;
  }

  auto properties =
      translation_resources.get_dialog(id.toStdString()).get_properties();
  for (auto kvp : properties) {
    map.insert(
          QString::fromStdString(kvp.first),
          QString::fromStdString(kvp.second));
  }
  return map;
}

/**
 * @brief Returns a property of a dialog with the specified id.
 * @param id The id to test.
 * @param key The key of the property to test.
 * @return the property value or an empty string if no exists.
 */
QString DialogsModel::get_translated_dialog_property(
    const QString& id, const QString& key) const {

  if (!translated_dialog_exists(id)) {
    return "";
  }
  const DialogData& data = translation_resources.get_dialog(id.toStdString());
  return QString::fromStdString(data.get_property(key.toStdString()));
}

/**
 * @brief Builds or rebuilds the indexed string tree.
 */
void DialogsModel::build_dialog_tree() {

  dialog_tree.clear();
  for (const auto& kvp : resources.get_dialogs()) {
    QString id = QString::fromStdString(kvp.first);
    dialog_tree.add_key(id);
  }
}

/**
 * @brief Removes all translated dialogs from the indexed tree.
 */
void DialogsModel::clear_translation_from_tree() {

  for (const auto& kvp : translation_resources.get_dialogs()) {
    QString id = QString::fromStdString(kvp.first);
    QString parent_id;
    int index;
    if (dialog_tree.can_remove_ref(id, parent_id, index)) {
      if (!dialog_exists(id)) {
        beginRemoveRows(id_to_index(parent_id), index, index);
        dialog_tree.remove_ref(id, false);
        endRemoveRows();
      } else {
        dialog_tree.remove_ref(id, true);
      }
    } else {
      dialog_tree.remove_ref(id, dialog_exists(id));
    }
  }
}
