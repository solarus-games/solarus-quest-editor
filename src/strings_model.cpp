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
#include "strings_model.h"
#include <QIcon>

using StringResources = Solarus::StringResources;

/**
 * @brief Creates a strings model.
 * @param quest The quest.
 * @param language_id Id of the language of strings to manage.
 * @param parent The parent object or nullptr.
 * @throws EditorException If the file could not be opened.
 */
StringsModel::StringsModel(
    const Quest& quest,
    const QString& language_id,
    QObject* parent) :
  QAbstractItemModel(parent),
  quest(quest),
  language_id(language_id),
  selection_model(this) {

  // Load the strings data file.
  QString path = quest.get_strings_path(language_id);
  if (!resources.import_from_file(path.toStdString())) {
    throw EditorException(tr("Cannot open strings data file '%1'").arg(path));
  }

  // Create the indexed tree.
  for (const auto& kvp : resources.get_strings()) {
    string_tree.add_key(QString::fromStdString(kvp.first));
  }
}

/**
 * @brief Returns the quest.
 */
const Quest& StringsModel::get_quest() const {
  return quest;
}

/**
 * @brief Returns the language id of the strings managed by this model.
 * @return The language id.
 */
QString StringsModel::get_language_id() const {
  return language_id;
}

/**
 * @brief Saves the strings to its data file.
 * @throws EditorException If the file could not be saved.
 */
void StringsModel::save() const {

  QString path = quest.get_strings_path(language_id);

  if (!resources.export_to_file(path.toStdString())) {
    throw EditorException(tr("Cannot save strings data file '%1'").arg(path));
  }
}

/**
 * @brief Returns the number of columns in the indexed tree.
 * @param parent Parent index.
 * @return The number of columns.
 */
int StringsModel::columnCount(const QModelIndex& /* parent */) const {
  return 2;
}

/**
 * @brief Returns the number of childs of a string key in the indexed tree.
 * @param parent Parent index.
 * @return The number of animations or number of directions from parent.
 */
int StringsModel::rowCount(const QModelIndex& parent) const {

  if (!parent.isValid()) {
    return string_tree.get_row_count();
  }

  if (parent.column() != 0) {
    return 0;
  }

  QString key = index_to_key(parent);
  if (!string_tree.key_exists(key)) {
    return 0;
  }
  return string_tree.get_row_count(key);;
}

/**
 * @brief Returns the index of an string key in the indexed tree.
 * @param row Row of the string key.
 * @param column Column of the string key.
 * @param parent Parent of the string key.
 * @return The corresponding index. Returns an invalid index if there is no
 * such the string key.
 */
QModelIndex StringsModel::index(
    int row, int column, const QModelIndex& parent) const {

  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  if (!parent.isValid()) {
    return key_to_index(string_tree.get_row_key(row), column);
  }

  QString key = index_to_key(parent);
  if (row >= string_tree.get_row_count(key)) {
    return QModelIndex();
  }

  return key_to_index(string_tree.get_row_key(row, key), column);
}

/**
 * @brief Returns the parent of the string key in the indexed tree.
 * @param index Index to get the parent of.
 * @return The parent index, or an invalid index if the string key has no parent.
 */
QModelIndex StringsModel::parent(const QModelIndex& model_index) const {

  if (!model_index.isValid()) {
    return QModelIndex();
  }

  QString key = string_tree.get_parent(index_to_key(model_index));
  return key_to_index(key);
}

/**
 * @brief Returns whether an string key in the indexed tree has any children.
 * @param parent The string key to test.
 * @return @c true if this string key has children.
 */
bool StringsModel::hasChildren(const QModelIndex& parent) const {

  return rowCount(parent) > 0;
}

/**
 * @brief Returns the data of a string key in the indexed tree for a given role.
 * @param index Index of the string key to get.
 * @param role The wanted role.
 * @return The data.
 */
QVariant StringsModel::data(const QModelIndex& model_index, int role) const {

  if (!model_index.isValid()) {
    return QVariant();
  }

  int column = model_index.column();
  QString key = index_to_key(model_index);
  if (column == 0) {
    if (string_tree.key_exists(key)) {
      switch (role) {
        case Qt::DisplayRole: return key.split(".").back(); break;

        case Qt::DecorationRole:
          if (!resources.has_string(key.toStdString())) {
            return QIcon(":/images/icon_folder_open.png");
          } else {
            if (string_tree.get_row_count(key) == 0) {
              return QIcon(":/images/icon_string.png");
            } else {
              return QIcon(":/images/icon_strings.png");
            }
          }
      }
    }
  } else if (column == 1 && string_exists(key) &&
             (role == Qt::DisplayRole || role == Qt::EditRole)) {
    return get_string(key);
  }

  return QVariant();
}

/**
 * @brief Returns the header info.
 * @param section Column or row number.
 * @param orientation Horizontal or vertical header orientation.
 * @param role The data under this role will be returned.
 * @return The header data.
 */
QVariant StringsModel::headerData(
    int section, Qt::Orientation orientation, int role) const {

  if (role == Qt::DisplayRole) {
    // Text of each header.
    switch (section) {
    case 0: return tr("Key");
    case 1: return tr("Value");
    }
  }

  return QVariant();
}

/**
 * @brief Returns the flags of an string key in the indexed tree.
 * @param index A string key index.
 * @return The string key flags.
 */
Qt::ItemFlags StringsModel::flags(const QModelIndex& index) const {

  Qt::ItemFlags flags =  Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  if (index.isValid() && index.column() == 1) {
    flags |= Qt::ItemIsEditable;
  }
  return flags;
}

/**
 * @brief Sets the data of an string key for a given role.
 * @param index Index of the string key to set.
 * @param value The new value to set.
 * @param role The role to change.
 * @parem @c true in case of success.
 */
bool StringsModel::setData(
    const QModelIndex& index, const QVariant& value, int role) {

  if (!index.isValid() || index.column() != 1 || role != Qt::EditRole) {
    return false;
  }

  emit set_value_requested(index_to_key(index), value.toString());
  return true;
}

bool StringsModel::is_valid_key(const QString& key) {
  return !key.isEmpty() && !key.startsWith(".") && !key.endsWith(".");
}

/**
 * @brief Returns the string key from specified model index.
 * @param index The model index.
 * @return The corresponding string key.
 */
QString StringsModel::index_to_key(const QModelIndex& index) const {

  if (!index.isValid()) {
    return "";
  }
  return *static_cast<QString*>(index.internalPointer());
}

/**
 * @brief Returns the model index of a specific string key.
 * @param key The string key.
 * @param column The column of the string key.
 * @return The correponding model index.
 */
QModelIndex StringsModel::key_to_index(const QString& key, int column) const {

  if (!string_tree.key_exists(key)) {
    return QModelIndex();
  }
  QString* internal_key = string_tree.get_internal_key(key);
  return createIndex(string_tree.get_index(key), column, internal_key);
}

/**
 * @brief Returns whether there exists a string with the specified key.
 * @param key The key to test.
 * @return @c true if such the specified string exists.
 */
bool StringsModel::string_exists(const QString& key) const {

  return resources.has_string(key.toStdString());
}

/**
 * @brief Returns the value of a string with the specified key.
 * @param key The key to test.
 * @return the value of the specified string or an empty string if no exists.
 */
QString StringsModel::get_string(const QString& key) const {

  if (!string_exists(key)) {
    return "";
  }
  return QString::fromStdString(resources.get_string(key.toStdString()));
}

/**
 * @brief Creates a new string.
 *
 * The index of the selection may change, since string are sorted
 * alphabetically.
 * Emits rowsAboutToBeInserted(), adds the new string
 * and then emits rowsInserted(), as required by QAbstractItemModel.
 *
 * Then, emits string_created().
 *
 * The newly created string is not initially selected.
 * The existing selection is preserved, though the index of many
 * strings can change.
 * The selection is cleared before the operations and restored after,
 * updated with the new index.
 *
 * @param key Key of the string to create.
 * @param value Value of the string to create.
 * @throws EditorException in case of error.
 */
void StringsModel::create_string(const QString& key, const QString& value) {

  // Make some checks first.
  if (!is_valid_key(key)) {
      throw EditorException(tr("Invalid string Key: %1").arg(key));
  }

  if (string_exists(key)) {
      throw EditorException(tr("String '%1' already exists").arg(key));
  }

  // Save and clear the selection since a lot of indexes may change.
  QString old_selection = get_selected_key();
  clear_selection();

  // Add to the strings file.
  resources.add_string(key.toStdString(), value.toStdString());

  // Update the indexed tree.
  QString parent_key;
  int index;
  if (string_tree.add_key(key, parent_key, index)) {
    // Notify people before restoring the selection, so that they have a
    // chance to know new indexes before receiving selection signals.
    beginInsertRows(key_to_index(parent_key), index, index);
    endInsertRows();
  } else {
    dataChanged(key_to_index(key), key_to_index(key, 1));
  }

  // Notify people.
  emit string_created(key);

  // Restore the selection.
  set_selected_key(old_selection);
}

/**
 * @brief Changes the value of the specified string.
 *
 * Emits dataChanged() and string_value_changed() if there is a change.
 *
 * @param key The key of the string ot change.
 * @param value The new value.
 */
void StringsModel::set_string(const QString& key, const QString& value) {

  if (get_string(key) == value) {
    // No change.
    return;
  }
  resources.get_string(key.toStdString()) = value.toStdString();

  // Notify people.
  QModelIndex index = key_to_index(key, 1);
  emit dataChanged(index, index);
  emit string_value_changed(key, value);
}

/**
 * @brief Changes the key of a string.
 *
 * The index of multiple strings may change, since they are sorted alphabetically.
 * In this case, emits rowsAboutToBeRemoved(), removes the string,
 * emits rowsRemoved() and rowsAboutToBeInserted(), adds the new string
 * and then emits rowsInserted(), as required by QAbstractItemModel.
 *
 * Then, emits string_key_changed(), no matter if the index has also changed.
 *
 * The selection is preserved, though the index of many strings can change.
 * The selection is cleared before the operations and restored after,
 * updated with the new indexes.
 *
 * @param keu Key of an existing string.
 * @param new_key The new key to set.
 * @return The new key of the pattern.
 * @throws EditorException in case of error.
 */
QString StringsModel::set_string_key(const QString& key, const QString& new_key) {

  if (new_key == key) {
    // Nothing to do.
    return key;
  }

  // Make some checks first.
  if (!string_exists(key)) {
      throw EditorException(tr("String '%1' no exists").arg(key));
  }

  if (string_exists(new_key)) {
      throw EditorException(tr("String '%1' already exists").arg(new_key));
  }

  if (!is_valid_key(new_key)) {
      throw EditorException(tr("Invalid string Key: %1").arg(new_key));
  }

  // Save and clear the selection since a lot of indexes may change.
  QString old_selection = get_selected_key();
  clear_selection();

  // Change in to the strings file.
  resources.set_string_key(key.toStdString(), new_key.toStdString());

  // Remove from the indexed tree.
  QString parent_key;
  int index;
  if (string_tree.can_remove_key(key, parent_key, index)) {

    // Call beginRemoveRows() as requested by QAbstractItemModel.
    beginRemoveRows(key_to_index(parent_key), index, index);
    string_tree.remove_key(key);
    endRemoveRows();
  } else if (string_tree.remove_key(key)) {
    dataChanged(key_to_index(key), key_to_index(key, 1));
  }

  // Add to the indexed tree.
  if (string_tree.add_key(new_key, parent_key, index)) {

    // Call beginInsertRows() as requested by QAbstractItemModel.
    beginInsertRows(key_to_index(parent_key), index, index);
    endInsertRows();
  } else {
    dataChanged(key_to_index(key), key_to_index(key, 1));
  }

  // Notify people.
  emit string_key_changed(key, new_key);

  // Restore the selection.
  set_selected_key(old_selection);
  return new_key;
}

/**
 * @brief Deletes a string.
 *
 * The index of multiple strings may change, since they are sorted alphabetically.
 * Emits rowsAboutToBeRemoved(), removes the string
 * and then emits rowsRemoved(), as required by QAbstractItemModel.
 *
 * Then, emits string_deleted().
 *
 * Except for the deleted string, the existing selection is preserved,
 * though the index of many strings can change.
 * The selection is cleared before the operations and restored after,
 * updated with the new indexes.
 *
 * @param key Key of the string to delete.
 * @throws EditorException in case of error.
 */
void StringsModel::delete_string(const QString& key) {

  // Make some checks first.
  if (!string_exists(key)) {
    throw EditorException(tr("Invalid string key: %1").arg(key));
  }

  // Save and clear the selection since a lot of indexes may change.
  QString old_selection = get_selected_key();
  clear_selection();

  // Delete from the strings file
  resources.remove_string(key.toStdString());

  // Remove from the indexed tree.
  QString parent_key;
  int index;
  if (string_tree.can_remove_key(key, parent_key, index)) {

    // Call beginRemoveRows() as requested by QAbstractItemModel.
    beginRemoveRows(key_to_index(parent_key), index, index);

    string_tree.remove_key(key);

    // Notify people before restoring the selection, so that they have a
    // chance to know new indexes before receiving selection signals.
    endRemoveRows();
  } else if (string_tree.remove_key(key)) {
    dataChanged(key_to_index(key), key_to_index(key, 1));
  }

  // Notify people.
  emit string_deleted(key);

  // Restore the selection.
  set_selected_key(old_selection);
}

/**
 * @brief Returns the selection model.
 * @return The selection info.
 */
QItemSelectionModel& StringsModel::get_selection_model() {
  return selection_model;
}

/**
 * @brief Returns whether no strings are selected.
 * @return @c true if the selection is empty.
 */
bool StringsModel::is_selection_empty() const {
  return selection_model.selection().isEmpty();
}

/**
 * @brief Returns the key of the selected string.
 * @return The selected string key.
 * Returns an empty string if the selection is empty.
 */
QString StringsModel::get_selected_key() const {

  QModelIndexList selected_indexes = selection_model.selectedIndexes();
  if (selected_indexes.isEmpty()) {
    return "";
  }
  return index_to_key(selected_indexes.first());
}

/**
 * @brief Selects a string and deselects all others.
 * @param key The key to select.
 */
void StringsModel::set_selected_key(const QString& key) {

  if (!string_tree.key_exists(key)) {
    return clear_selection();
  }

  QItemSelection selection(key_to_index(key), key_to_index(key, 1));
  selection_model.select(selection, QItemSelectionModel::ClearAndSelect);
}

/**
 * @brief Deselects all selected items.
 */
void StringsModel::clear_selection() {
  selection_model.clear();
}
