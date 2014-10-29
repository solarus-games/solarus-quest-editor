/*
 * Copyright (C) 2014 Christopho, Solarus - http://www.solarus-games.org
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
#include "gui/quest_files_model.h"
#include "editor_exception.h"
#include "quest.h"
#include <QFileSystemModel>

/**
 * @brief Creates a quest files model.
 * @param parent Path of the quest to represent.
 */
QuestFilesModel::QuestFilesModel(Quest& quest):
  QSortFilterProxyModel(nullptr),
  quest(quest),
  source_model(new QFileSystemModel) {

  // Watch changes on the filesystem.
  source_model->setRootPath(quest.get_data_path());  // Only watch changes in the data directory.
  source_model->setReadOnly(false);
  setSourceModel(source_model);

  // Watch changes in resources.
  connect(&quest.get_resources(), SIGNAL(element_description_changed(ResourceType, const QString&, const QString&)),
          this, SLOT(resource_element_description_changed(ResourceType, const QString&, const QString&)));
}

/**
 * @brief Returns the quest represented by this model.
 * @return The quest.
 */
Quest& QuestFilesModel::get_quest() {
  return quest;
}

/**
 * @brief Returns the index of the root item of the quest.
 * @return The root index.
 */
QModelIndex QuestFilesModel::get_quest_root_index() const {

  return mapFromSource(source_model->index(quest.get_root_path()));
}

/**
 * @brief Returns the number of columns in the model.
 *
 * Reimplemented to set custom columns.
 *
 * @param parent Parent index.
 * @return The number of columns
 */
int QuestFilesModel::columnCount(const QModelIndex& /* parent */) const {

  // File, Description, Type.
  return 3;
}

/**
 * @brief Returns the number of rows under a parent item.
 *
 * Reimplemented to add rows with declared resource elements that are missing
 * on the filesystem.
 *
 * @param parent Parent index.
 * @return The number of rows under this parent.
 */
int QuestFilesModel::rowCount(const QModelIndex& parent) const {

  // Get the number of rows from the filesytem (filtered)
  // and add rows for resource element whose file is missing.
  int existing_files_count = QSortFilterProxyModel::rowCount(parent);
  int missing_files_count = get_missing_resource_elements(parent).size();

  return existing_files_count + missing_files_count;
}

/**
 * @brief TODO
 * @param row
 * @param column
 * @param parent
 * @return
 */
QModelIndex QuestFilesModel::index(int row, int column, const QModelIndex& parent) const {
    if (row < 0 || column < 0) {
      return QModelIndex();
    }
    QModelIndex candidate = QSortFilterProxyModel::index(row, column, parent);
    if (!candidate.isValid()) {
      return createIndex(row, column, 42);  // TODO
    }
    return candidate;
}

/**
 * @brief TODO
 * @param index
 * @return
 */
QModelIndex QuestFilesModel::parent(const QModelIndex& index) const {
  if (index.internalId() == 42) {
    return get_file_index(quest.get_resource_path(ResourceType::LANGUAGE));
  }
  return QSortFilterProxyModel::parent(index);
}

/**
 * @brief TODO
 * @param row
 * @param column
 * @param idx
 * @return
 */
QModelIndex QuestFilesModel::sibling(int row, int column, const QModelIndex& idx) const {
  return index(row, column, parent(idx));
}

/**
 * @brief Returns whether an item has any children.
 * @param parent The item to test.
 * @return @c true if this item has children.
 */
bool QuestFilesModel::hasChildren(const QModelIndex& parent) const {

  QString file_path = get_file_path(parent);
  ResourceType resource_type;
  QString element_id;

  if (quest.is_resource_element(file_path, resource_type, element_id) &&
      resource_type == ResourceType::LANGUAGE) {
    // Remove the subtree of languages.
    return false;
  }

  return QSortFilterProxyModel::hasChildren(parent);
}

/**
 * @brief TODO
 * @param proxy_index
 * @return
 */
QModelIndex QuestFilesModel::mapToSource(const QModelIndex& proxy_index) const {

  if (proxy_index.internalId() == 42) {
    // This item does not exist in the source model
    // (it was added by the proxy).
    return QModelIndex();
  }
  return QSortFilterProxyModel::mapToSource(proxy_index);
}

/**
 * @brief Returns the flags of an item.
 * @param index An item index.
 * @return The item flags.
 */
Qt::ItemFlags QuestFilesModel::flags(const QModelIndex& index) const {

  QString file_path = get_file_path(index);
  ResourceType resource_type;
  QString element_id;
  Qt::ItemFlags flags =  Qt::ItemIsSelectable | Qt::ItemIsEnabled;

  switch (index.column()) {

  case FILE_COLUMN:  // File name.

    if (quest.is_resource_element(file_path, resource_type, element_id) &&
        resource_type == ResourceType::LANGUAGE) {
      // Ignore the subtree of languages.
      flags |= Qt::ItemNeverHasChildren;
    }
    return flags;

  case DESCRIPTION_COLUMN:  // Resource description.

    if (quest.is_resource_element(file_path, resource_type, element_id)) {
      // The description column of a resource element can be modified.
      return flags | Qt::ItemIsEditable;
    }
    return flags;
  }

  return flags;
}

/**
 * @brief Returns the header info.
 * @param section Column or row number.
 * @param orientation Horizontal or vertical header orientation.
 * @param role The data under this role will be returned.
 * @return The header data.
 */
QVariant QuestFilesModel::headerData(int section, Qt::Orientation orientation, int role) const {

  switch (role) {

  case Qt::DisplayRole:
    // Text of each header.
    switch (section) {

    case FILE_COLUMN:
      return tr("Resource");

    case DESCRIPTION_COLUMN:
      return tr("Description");

    case TYPE_COLUMN:
      return tr("Type");
    }
    return QVariant();
  }

  // For other roles, rely on the standard header settings.
  return QSortFilterProxyModel::headerData(section, orientation, role);
}

/**
 * @brief Returns the data of an item for a given role.
 * @param index Index of the item to get.
 * @param role The data under this role will be returned.
 * @return The data.
 */
QVariant QuestFilesModel::data(const QModelIndex& index, int role) const {

  const QuestResources& resources = quest.get_resources();
  ResourceType resource_type;
  QString element_id;

  QString path;

  // TODO move to get_file_path()

  if (index.internalId() != 42) {
    // The item is a file that exists on a filesystem.
    QModelIndex source_index = mapToSource(index);
    QModelIndex file_source_index = source_model->index(
          source_index.row(), FILE_COLUMN, source_index.parent());
    path = source_model->filePath(file_source_index);
  }
  else {
    // The file represented by this item does not exist.
    // It must be a declared resource element whose file is missing
    // (see how rowCount() adds rows for missing elements).
    const QModelIndex& parent_index = index.parent();
    QString parent_path = get_file_path(parent_index);
    if (!quest.is_resource_path(parent_path, resource_type) &&
        !quest.is_in_resource_path(parent_path, resource_type)) {
      // No resource elements can exist here.
      return QVariant();
    }

    // Get all missing element ids in this directory to know our index.
    QStringList missing_element_ids = get_missing_resource_elements(parent_index);
    int num_existing = QSortFilterProxyModel::rowCount(parent_index);
    int index_in_missing = index.row() - num_existing;
    if (index_in_missing < 0 || index_in_missing >= missing_element_ids.size()) {
      // Something is wrong.
      return QVariant();
    }
    QString element_id = missing_element_ids[index_in_missing];
    path = quest.get_resource_element_path(resource_type, element_id);
  }
  QString file_name = QFileInfo(path).baseName();

  switch (role) {

  case Qt::DisplayRole:
    // Text of each file item.

    switch (index.column()) {

    case FILE_COLUMN:  // File name.
      if (is_quest_data_index(index)) {
        // Data directory: show the quest name instead of "data".
        return quest.get_name();
      }

      if (quest.is_resource_element(path, resource_type, element_id)) {
        // A resource element: show its id (remove the extension).
        if (resource_type != ResourceType::LANGUAGE) {
          return QFileInfo(file_name).completeBaseName();
        }
      }
      return file_name;

    case DESCRIPTION_COLUMN:  // Resource element description.

      if (!quest.is_resource_element(path, resource_type, element_id)) {
        return QVariant();
      }
      return resources.get_description(resource_type, element_id);

    case TYPE_COLUMN:  // Type
      if (is_quest_data_index(index)) {
        // Quest data directory (top-level item).
        return tr("Quest");
      }

      if (path == quest.get_main_script_path()) {
        // main.lua
        return tr("Main Lua script");
      }

      if (quest.is_resource_path(path, resource_type)) {
        // A resource element folder.
        return resources.get_directory_friendly_name(resource_type);
      }

      if (quest.is_resource_element(path, resource_type, element_id)) {
        // A declared resource element.
        return resources.get_friendly_name(resource_type);
      }

      if (quest.is_script(path)) {
        // An arbitrary Lua script.
        return tr("Lua script");
      }

      // Not a file managed by Solarus.
      return QVariant();
    }

  case Qt::EditRole:
    // Editable file name.
    switch (index.column()) {

    case FILE_COLUMN:  // File name.
      return file_name;

    case DESCRIPTION_COLUMN:
      // The resource element description can be edited.
      if (!quest.is_resource_element(path, resource_type, element_id)) {
        return QVariant();
      }
      return resources.get_description(resource_type, element_id);
    }

  case Qt::DecorationRole:
    // Icon.
    if (index.column() == FILE_COLUMN) {
      return QIcon(":/images/" + get_quest_file_icon_name(index));
    }
    return QVariant();  // No icon in other columns.

  case Qt::ToolTipRole:
    // Tooltip.
    if (index.column() == FILE_COLUMN) {
      return get_quest_file_tooltip(index);
    }
    return QVariant();  // No tooltip in other columns.

  case Qt::TextAlignmentRole:
    // Remove the alignment done by QFileSystemModel.
    return Qt::AlignLeft;
  }

  // For other roles, rely on the standard settings.
  if (index.internalId() == 42) {  // TODO
    return QVariant();
  }
  return QSortFilterProxyModel::data(index, role);
}

/**
 * @brief Sets the data of an item for a given role.
 * @param index Index of the item to set.
 * @param value The new value to set.
 * @param role The role to change.
 * @parem @c true in case of success.
 */
bool QuestFilesModel::setData(
    const QModelIndex& index, const QVariant& value, int role) {

  if (index.column() != DESCRIPTION_COLUMN) {
    // Only the description column is editable.
    return false;
  }

  if (role != Qt::EditRole) {
    return false;
  }

  QString file_path = get_file_path(index);
  ResourceType resource_type;
  QString element_id;
  if (!quest.is_resource_element(file_path, resource_type, element_id)) {
    return false;
  }

  try {
    quest.get_resources().set_description(resource_type, element_id, value.toString());
    quest.get_resources().save();
    emit dataChanged(index, index);
    return true;
  }
  catch (const EditorException& ex) {
    ex.print_message();
    return false;
  }
}

/**
 * @brief Slot called when the description of a resource element changes.
 * @param resource_type A type of resource.
 * @param element_id Id of the element whose description has changed.
 * @param description The new description.
 */
void QuestFilesModel::resource_element_description_changed(
    ResourceType resource_type, const QString& element_id, const QString& /* description */) {

  QModelIndex index = get_file_index(quest.get_resource_element_path(resource_type, element_id));
  index = this->index(index.row(), DESCRIPTION_COLUMN, index.parent());
  emit dataChanged(index, index);
}

/**
 * @brief Returns an appropriate icon for the specified quest file.
 * @param index Index of a file item in the model.
 * @return An appropriate icon name to represent this file, relative to the
 * images directory of resources.
 */
QString QuestFilesModel::get_quest_file_icon_name(const QModelIndex& index) const {

  QString file_path = get_file_path(index);
  ResourceType resource_type;
  QString element_id;

  // Quest data directory.
  if (is_quest_data_index(index)) {
    return "icon_solarus.png";
  }

  // Resource element (possibly a directory for languages).
  if (quest.is_resource_element(file_path, resource_type, element_id)) {
    const std::string& resource_type_name =
        Solarus::QuestResources::get_resource_type_name(resource_type);
    return "icon_resource_" +
        QString::fromStdString(resource_type_name) +
        ".png";
  }

  // Directory icon.
  if (quest.is_dir(file_path)) {

    if (quest.is_resource_path(file_path, resource_type)) {
      const std::string& resource_type_name =
          Solarus::QuestResources::get_resource_type_name(resource_type);
      return "icon_folder_open_" +
          QString::fromStdString(resource_type_name) +
          ".png";
    }

    return "icon_folder_open.png";
  }

  // Lua script icon.
  if (quest.is_script(file_path)) {
    return "icon_script.png";
  }

  // Generic icon for a file not known by the quest.
  return "icon_file_unknown.png";
}

/**
 * @brief Returns an appropriate tooltip for the specified quest file.
 * @param index Index of a file item in the model.
 * @return An appropriate tooltip for this file item.
 */
QString QuestFilesModel::get_quest_file_tooltip(const QModelIndex& index) const {

  QModelIndex source_index = mapToSource(index);
  QString path = source_model->filePath(source_index);
  QString file_name = QFileInfo(path).fileName();
  ResourceType resource_type;
  QString element_id;

  // Show a tooltip for resource elements because their item text is different
  // from the physical file name.
  if (quest.is_resource_element(path, resource_type, element_id)) {
    return file_name;
  }

  return "";
}

/**
 * @brief Compares two items for sorting purposes.
 * @param left An item index in the source model.
 * @param right Another item index in the source model.
 * @return \c true if the value of the first item is less than the second one.
 */
bool QuestFilesModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {

  // Directories are before regular files.
  if (source_model->isDir(left) &&
      !source_model->isDir(right)) {
    return true;
  }

  if (!source_model->isDir(left) &&
      source_model->isDir(right)) {
    return false;
  }

  return QSortFilterProxyModel::lessThan(left, right);
}

/**
 * @brief Returns whether a source row should be included in the model.
 * @param source_row A row in the source model.
 * @param source_parent Parent index of the row in the source model.
 * @return \c true to keep the row, \c false to filter it out.
 */
bool QuestFilesModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {

  QModelIndex source_index = source_model->index(source_row, FILE_COLUMN, source_parent);
  QString file_name = source_model->fileName(source_index);
  QString file_path = source_model->filePath(source_index);

  if (source_model->index(source_model->rootPath()).parent() == source_parent) {
    // This is a top-level item: only keep the quest data directory.
    if (file_path == quest.get_data_path()) {
      return true;
    }
    return false;
  }

  if (source_model->isDir(source_index)) {
    // Keep all other directories.
    return true;
  }

  const QString lua_extension = ".lua";
  if (file_name.endsWith(lua_extension)) {
    // Keep all .lua scripts except map scripts.
    QString file_path_dat = file_path.replace(file_path.lastIndexOf(lua_extension), lua_extension.size(), ".dat");
    ResourceType resource_type;
    QString element_id;
    if (quest.is_resource_element(file_path_dat, resource_type, element_id) &&
        resource_type == ResourceType::MAP) {
      return false;
    }

    return true;
  }

  // Keep resources, and also files that could be resources
  // but are not declared in the resource list yet.
  ResourceType resource_type;
  QString element_id;
  if (quest.is_potential_resource_element(file_path, resource_type, element_id)) {
    return true;
  }

  // File not known by the quest.
  return false;
}

/**
 * @brief Returns whether a source index is the index of the quest data directory.
 *
 * This function returns @c true for all columns of the data directory row.
 *
 * @param source_index An index in the model.
 * @return \c true if this is the quest data directory.
 */
bool QuestFilesModel::is_quest_data_index(const QModelIndex& index) const {

  return get_file_path(index) == quest.get_data_path();
}

/**
 * @brief Returns the path of the file at the specified index.
 *
 * Only the row of the index is considered, so the result is the same for all
 * columns.
 *
 * @param index Index of an item in this model.
 * @return The corresponding path.
 */
QString QuestFilesModel::get_file_path(const QModelIndex& index) const {

  // TODO support missing resource files
  if (index.internalId() == 42) {
    return "";
  }

  QModelIndex source_index = mapToSource(index);
  QModelIndex file_source_index = source_model->index(
        source_index.row(), FILE_COLUMN, source_index.parent());
  return source_model->filePath(file_source_index);
}

/**
 * @brief Returns the index of the specified path.
 *
 * The column of the returned index is always FILE_COLUMN.
 *
 * @param path Path of a file.
 * @return The corresponding index. An invalid index is returned if the path
 * does not exist in the model.
 */
QModelIndex QuestFilesModel::get_file_index(const QString& path) const {

  // TODO support missing resource files

  return mapFromSource(source_model->index(path, 0));
}

/**
 * @brief Returns the ids of resource elements that are declared under an item
 * but whose files are missing on the filesystem.
 * @param parent An index in the model. If this is a directory, resources
 * directly under this directory are checked. Otherwise, returns an empty list.
 * @return The ids of resource elements whose file is missing under the item.
 */
QStringList QuestFilesModel::get_missing_resource_elements(const QModelIndex& parent) const {

  QString parent_path = get_file_path(parent);
  if (!quest.is_dir(parent_path)) {
    // Parent is not a directory: nothing more to do.
    return QStringList();
  }

  ResourceType resource_type;
  QString element_id;
  if (quest.is_resource_element(parent_path, resource_type, element_id) &&
      resource_type == ResourceType::LANGUAGE) {
    // Ignore the subtree of languages.
    return QStringList();
  }

  if (!quest.is_resource_path(parent_path, resource_type) &&
      !quest.is_in_resource_path(parent_path, resource_type)) {
    // Parent is not a resource directory: we will not find resources there.
    return QStringList();
  }

  // Get all declared elements of this resource type that are directly in
  // the directory.
  QStringList missing_element_ids;
  QStringList element_ids = quest.get_resources().get_elements(resource_type);
  for (QString element_id : element_ids) {
    QString current_path = quest.get_resource_element_path(resource_type, element_id);
    QDir current_parent_dir(current_path);
    if (!current_parent_dir.cdUp()) {
      // Even the parent directory of the element does not exist.
      continue;
    }

    QString current_parent_path = current_parent_dir.path();
    if (current_parent_path != parent_path) {
      // This resource element is not (or not directly) in the directory.
      continue;
    }

    // The current resource element is declared in this directory.
    // Check that its file exists.
    if (!quest.exists(quest.get_resource_element_path(resource_type, element_id))) {
      missing_element_ids << element_id;
    }
  }

  return missing_element_ids;
}
