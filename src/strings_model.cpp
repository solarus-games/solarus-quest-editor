/*
 * Copyright (C) 2014-2017 Christopho, Solarus - http://www.solarus-games.org
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

namespace SolarusEditor {

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
  build_string_tree();
}

/**
 * @brief Destructor.
 */
StringsModel::~StringsModel() {
  string_tree.clear();
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
  return 3;
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
          if (string_exists(key)) {
            if (has_missing_translation(key)) {
              return QIcon(":/images/icon_strings_missing.png");
            } else if (string_tree.get_row_count(key) == 0) {
              return QIcon(":/images/icon_string.png");
            }
            return QIcon(":/images/icon_strings.png");
          }
          else if (has_missing_translation(key)) {
            if (string_tree.get_row_count(key) == 0) {
              return QIcon(":/images/icon_string_missing.png");
            } else if (translated_string_exists(key)) {
              return QIcon(":/images/icon_strings_missing.png");
            }
            return QIcon(":/images/icon_folder_open_missing.png");
          }
          return QIcon(":/images/icon_folder_open.png");
      }
    }
  } else if (column == 1 && string_exists(key) &&
             (role == Qt::DisplayRole || role == Qt::EditRole)) {
    return get_string(key);
  } else if (column == 2 && role == Qt::DisplayRole) {
    if (translated_string_exists(key)) {
      return get_translated_string(key);
    }
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

  Q_UNUSED(orientation);

  if (role == Qt::DisplayRole) {
    // Text of each header.
    switch (section) {
    case 0: return tr("Key");
    case 1: return tr("Value");
    case 2:
      if (!translation_id.isEmpty()) {
        return tr("Translation (%1)").arg(translation_id);
      }
      return tr("Translation");
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
 * @brief Returns whether exists key starts with a specified prefix.
 * @param prefix The prefix to test.
 * @return @c true if a key that starts with the specified prefix exists.
 */
bool StringsModel::prefix_exists(const QString& prefix) const {

  for (const auto& kvp : resources.get_strings()) {
    if (QString::fromStdString(kvp.first).startsWith(prefix)) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Returns all the existing keys that start with the specified prefix.
 * @param prefix The prefix to test.
 * @return The list of all keys that starts with the specified prefix.
 */
QStringList StringsModel::get_keys(const QString& prefix) const {

  QStringList list;
  for (const auto& kvp : resources.get_strings()) {
    QString key = QString::fromStdString(kvp.first);
    if (key.startsWith(prefix)) {
      list.push_back(key);
    }
  }
  return list;
}

/**
 * @brief Returns the value of a string with the specified key.
 * @param key The key to test.
 * @return The value of the specified string or an empty string if it does not exist.
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
      throw EditorException(tr("Invalid string id: '%1'").arg(key));
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
    dataChanged(key_to_index(key), key_to_index(key, 2));
  }

  // Notify people.
  emit string_created(key);

  // Restore the selection.
  set_selected_key(old_selection);
}

/**
 * @brief Returns whether strings with a key prefix can be duplicated.
 * @param prefix[in] The prefix key of strings to duplicate.
 * @param new_prefix[in] The new prefix key to use.
 * @param key[out] The key that already exists in case of error.
 * @return @c true if strings can be duplicated.
 */
bool StringsModel::can_duplicate_strings(
  const QString& prefix, const QString& new_prefix, QString& key) {

  const QStringList& keys = get_keys(prefix);
  for (QString prefixed_key : keys) {

    prefixed_key.replace(QRegExp(QString("^") + prefix), new_prefix);
    if (string_exists(prefixed_key)) {
      key = prefixed_key;
      return false;
    }
  }
  return true;
}

/**
 * @brief Duplicates string(s).
 * @param id Id of the dialog to duplicate.
 * @param new_id Id of the duplicate dialog.
 * @throws EditorException in case of error.
 */
void StringsModel::duplicate_strings(
  const QString& prefix, const QString& new_prefix) {

  // Check if strings can be duplicated.
  QString key;
  if (!can_duplicate_strings(prefix, new_prefix, key)) {
    throw EditorException(tr("String '%1' already exists").arg(key));
  }

  // Duplicate strings.
  const QStringList& keys = get_keys(prefix);
  for (QString key : keys) {
    QString value = get_string(key);
    key.replace(QRegExp(QString("^") + prefix), new_prefix);
    create_string(key, value);
  }
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
 * @param key Key of an existing string.
 * @param new_key The new key to set.
 * @return The new key of the string.
 * @throws EditorException in case of error.
 */
QString StringsModel::set_string_key(const QString& key, const QString& new_key) {

  if (new_key == key) {
    // Nothing to do.
    return key;
  }

  // Make some checks first.
  if (!string_exists(key)) {
    throw EditorException(tr("String '%1' does not exist").arg(key));
  }

  if (string_exists(new_key)) {
    throw EditorException(tr("String '%1' already exists").arg(new_key));
  }

  if (!is_valid_key(new_key)) {
    throw EditorException(tr("Invalid string id: '%1'").arg(new_key));
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
    dataChanged(key_to_index(key), key_to_index(key, 2));
  }

  // Add to the indexed tree.
  if (string_tree.add_key(new_key, parent_key, index)) {

    // Call beginInsertRows() as requested by QAbstractItemModel.
    beginInsertRows(key_to_index(parent_key), index, index);
    endInsertRows();
  } else {
    dataChanged(key_to_index(key), key_to_index(key, 2));
  }

  // Notify people.
  emit string_key_changed(key, new_key);

  // Restore the selection.
  set_selected_key(old_selection);
  return new_key;
}

/**
 * @brief Returns whether the prefix of string keys can be changed.
 * @param old_prefix[in] The prefix key of strings to change.
 * @param new_prefix[in] The new prefix to set.
 * @param key[out] The key that already exists in case of error.
 * @return @c true if the prefix can be changed.
 */
bool StringsModel::can_set_string_key_prefix(
    const QString& old_prefix, const QString& new_prefix, QString& key) {

  const QStringList& keys = get_keys(old_prefix);
  for (QString prefixed_key : keys) {

    prefixed_key.replace(QRegExp(QString("^") + old_prefix), new_prefix);
    if (string_exists(prefixed_key)) {
      key = prefixed_key;
      return false;
    }
  }
  return true;
}

/**
 * @brief Changes the prefix of string keys.
 * @param old_prefix The prefix key of strings to change.
 * @param new_prefix The new prefix to set.
 * @return The new keys of the string.
 * @throws EditorException in case of error.
 */
QList<QPair<QString, QString>> StringsModel::set_string_key_prefix(
    const QString& old_prefix, const QString& new_prefix) {

  // Check if the prefix can be changed.
  QString key;
  if (!can_set_string_key_prefix(old_prefix, new_prefix, key)) {
    throw EditorException(tr("String '%1' already exists").arg(key));
  }

  // change the string keys.
  QList<QPair<QString, QString>> list;
  const QStringList& old_keys = get_keys(old_prefix);
  for (const QString& old_key : old_keys) {

    QString new_key = old_key;
    new_key.replace(QRegExp(QString("^") + old_prefix), new_prefix);
    list.push_back(
      QPair<QString, QString>(old_key, set_string_key(old_key, new_key)));
  }
  return list;
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
    throw EditorException(tr("Invalid string id: '%1'").arg(key));
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
    dataChanged(key_to_index(key), key_to_index(key, 2));
  }

  // Notify people.
  emit string_deleted(key);

  // Restore the selection.
  set_selected_key(old_selection);
}

/**
 * @brief Removes all strings with a specific prefix.
 * @param prefix The prefix key of strings to remove.
 * @return The list of all removed strings (key, value).
 * @throws EditorException in case of error.
 */
QList<QPair<QString, QString>> StringsModel::delete_prefix(const QString& prefix) {

  QList<QPair<QString, QString>> list;
  const QStringList& keys = get_keys(prefix);
  for (const QString& key : keys) {
    list.push_back(qMakePair(key, get_string(key)));
    delete_string(key);
  }
  return list;
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

  QItemSelection selection(key_to_index(key), key_to_index(key, 2));
  selection_model.select(selection, QItemSelectionModel::ClearAndSelect);
}

/**
 * @brief Deselects all selected items.
 */
void StringsModel::clear_selection() {
  selection_model.clear();
}

/**
 * @brief Returns the language id of the current translation.
 * @return The language if of the translation.
 */
QString StringsModel::get_translation_id() const {
  return translation_id;
}

/**
 * @brief Changes the language of the current translation.
 * @param language_id The language id of the translation.
 */
void StringsModel::set_translation_id(const QString& language_id) {

  if (language_id == translation_id) {
    return;
  }
  translation_id = language_id;
  reload_translation();
}

/**
 * @brief Removes the current translation.
 */
void StringsModel::clear_translation() {

  if (translation_id.isEmpty()) {
    return;
  }

  translation_id = "";
  clear_translation_from_tree();
  translation_resources.clear();
  headerDataChanged(Qt::Horizontal, 2, 2);
}

/**
 * @brief Reload the current translation.
 */
void StringsModel::reload_translation() {

  clear_translation_from_tree();

  QString path = quest.get_strings_path(translation_id);
  translation_resources.clear();
  if (!translation_resources.import_from_file(path.toStdString())) {
    translation_id = "";
    throw EditorException(tr("Cannot open strings data file '%1'").arg(path));
  }

  for (const auto& kvp : translation_resources.get_strings()) {
    QString key = QString::fromStdString(kvp.first);
    QString parent_key;
    int index;
    if (string_tree.add_ref(key, parent_key, index)) {
      beginInsertRows(key_to_index(parent_key), index, index);
      endInsertRows();
    } else {
      QModelIndex model_index = key_to_index(key, 2);
      dataChanged(model_index, model_index);
    }
  }

  headerDataChanged(Qt::Horizontal, 2, 2);
}

/**
 * @brief Returns whether a translated string exists.
 * @param key The key of the string.
 * @return @c true if the translated string exists.
 */
bool StringsModel::translated_string_exists(const QString& key) const {

  return translation_resources.has_string(key.toStdString());
}

/**
 * @brief Returns all the translated keys that start with the specified prefix.
 * @param prefix The prefix to test.
 * @return The list of all translated keys that starts with the prefix.
 */
QStringList StringsModel::get_translated_keys(const QString& prefix) const {

  QStringList list;
  for (const auto& kvp : translation_resources.get_strings()) {
    QString key = QString::fromStdString(kvp.first);
    if (key.startsWith(prefix)) {
      list.push_back(key);
    }
  }
  return list;
}

/**
 * @brief Returns whether string or sub string has translation and don't exists.
 * @param key The key of the string.
 * @return @c true if the string or sub string has tanslation and don't exists.
 */
bool StringsModel::has_missing_translation(const QString& key) const {

  if (!string_exists(key) && translated_string_exists(key)) {
    return true;
  }

  const QStringList& sub_keys = get_translated_keys(key + ".");
  for (const QString& sub_key : sub_keys) {
    if (!string_exists(sub_key)) {
      return true;
    }
  }

  return false;
}

/**
 * @brief Returns a translated string.
 * @param key The key of the string.
 * @return The translated string or an empty string if it does not exist.
 */
QString StringsModel::get_translated_string(const QString& key) const {

  if (!translated_string_exists(key)) {
    return "";
  }
  return QString::fromStdString(
        translation_resources.get_string(key.toStdString()));
}

/**
 * @brief Builds or rebuilds the indexed string tree.
 */
void StringsModel::build_string_tree() {

  string_tree.clear();
  for (const auto& kvp : resources.get_strings()) {
    QString key = QString::fromStdString(kvp.first);
    string_tree.add_key(key);
  }
}

/**
 * @brief Removes all translated string from the indexed string tree.
 */
void StringsModel::clear_translation_from_tree() {

  for (const auto& kvp : translation_resources.get_strings()) {
    QString key = QString::fromStdString(kvp.first);
    QString parent_key;
    int index;
    if (string_tree.can_remove_ref(key, parent_key, index)) {
      if (!string_exists(key)) {
        beginRemoveRows(key_to_index(parent_key), index, index);
        string_tree.remove_ref(key, false);
        endRemoveRows();
      } else if (string_tree.remove_ref(key, true)) {
        QModelIndex model_index = key_to_index(key, 2);
        dataChanged(model_index, model_index);
      }
    } else if (string_tree.remove_ref(key, string_exists(key))) {
      QModelIndex model_index = key_to_index(key, 2);
      dataChanged(model_index, model_index);
    }
  }
}

}
