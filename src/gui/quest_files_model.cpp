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
#include "quest.h"
#include "quest_resources.h"
#include "solarus/QuestResources.h"
#include <QFileSystemModel>

/**
 * @brief Creates a quest files model.
 * @param parent Path of the quest to represent.
 */
QuestFilesModel::QuestFilesModel(Quest& quest):
  QSortFilterProxyModel(nullptr),
  quest(quest),
  source_model(new QFileSystemModel) {

  source_model->setRootPath(quest.get_data_path());  // Only watch changes in the data directory.
  source_model->setReadOnly(false);
  setSourceModel(source_model);
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
 * @brief Returns the flags of an item.
 * @param index An item index.
 * @return The item flags.
 */
Qt::ItemFlags QuestFilesModel::flags(const QModelIndex& index) const {

  QString file_path = get_file_path(index);
  Solarus::ResourceType resource_type;
  QString element_id;
  Qt::ItemFlags flags =  Qt::ItemIsSelectable | Qt::ItemIsEnabled;

  switch (index.column()) {

  case FILE_COLUMN:  // File name.

    if (quest.is_resource_element(file_path, resource_type, element_id) &&
        resource_type == Solarus::ResourceType::LANGUAGE) {
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
 * @brief Returns whether an item has any children.
 * @param parent The item to test.
 * @return @c true if this item has children.
 */
bool QuestFilesModel::hasChildren(const QModelIndex& parent) const {

  QString file_path = get_file_path(parent);
  Solarus::ResourceType resource_type;
  QString element_id;

  if (quest.is_resource_element(file_path, resource_type, element_id) &&
      resource_type == Solarus::ResourceType::LANGUAGE) {
    // Remove the subtree of languages.
    return false;
  }

  return QSortFilterProxyModel::hasChildren(parent);
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

  QModelIndex source_index = mapToSource(index);
  QModelIndex file_source_index = source_model->index(
        source_index.row(), FILE_COLUMN, source_index.parent());
  QString file_name = source_model->fileName(file_source_index);
  QString file_path = source_model->filePath(file_source_index);
  Solarus::ResourceType resource_type;
  QString element_id;

  switch (role) {

  case Qt::DisplayRole:
    // Text of each file item.

    switch (index.column()) {

    case FILE_COLUMN:  // File name.
      if (is_quest_data_index(source_index)) {
        // Data directory: show the quest name instead of "data".
        return quest.get_name();
      }

      if (quest.is_resource_element(file_path, resource_type, element_id)) {
        // A resource element: show its id (remove the extension).
        if (resource_type != Solarus::ResourceType::LANGUAGE) {
          return QFileInfo(file_name).completeBaseName();
        }
      }
      return file_name;

    case DESCRIPTION_COLUMN:  // Resource element description.

      if (!quest.is_resource_element(file_path, resource_type, element_id)) {
        return QVariant();
      }
      return quest.get_resources().get_description(resource_type, element_id);

    case TYPE_COLUMN:  // Type
      if (is_quest_data_index(source_index)) {
        // Quest data directory (top-level item).
        return tr("Quest");
      }

      if (file_path == quest.get_main_script_path()) {
        // main.lua
        return tr("Main Lua script");
      }

      if (quest.is_resource_path(file_path, resource_type)) {
        // A resource element folder.
        return tr("%1 folder %2").
            arg(QuestResources::get_friendly_name(resource_type)).arg("");
      }

      if (quest.is_resource_element(file_path, resource_type, element_id)) {
        // A declared resource element.
        return QuestResources::get_friendly_name(resource_type);
      }

      if (file_name.endsWith(".lua")) {
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
      if (!quest.is_resource_element(file_path, resource_type, element_id)) {
        return QVariant();
      }
      return quest.get_resources().get_description(resource_type, element_id);
    }

  case Qt::DecorationRole:
    // Icon.
    if (index.column() == FILE_COLUMN) {
      return QIcon(":/images/" + get_quest_file_icon_name(source_index));
    }
    return QVariant();  // No icon in other columns.

  case Qt::TextAlignmentRole:
    // Remove the alignment done by QFileSystemModel.
    return Qt::AlignLeft;
  }

  // For other roles, rely on the standard settings.
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
  Solarus::ResourceType resource_type;
  QString element_id;
  if (!quest.is_resource_element(file_path, resource_type, element_id)) {
    return false;
  }

  quest.get_resources().set_description(resource_type, element_id, value.toString());
  quest.get_resources().save();
  emit dataChanged(index, index);

  return true;
}

/**
 * @brief Returns an appropriate icon for the specified quest file.
 * @param index Index of a file item in the source model.
 * @return An appropriate icon name to represent this file.
 */
QString QuestFilesModel::get_quest_file_icon_name(const QModelIndex& source_index) const {

  QString file_name = source_model->fileName(source_index);
  QString file_path = source_model->filePath(source_index);
  Solarus::ResourceType resource_type;
  QString element_id;

  // Quest data directory.
  if (is_quest_data_index(source_index)) {
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
  if (source_model->isDir(source_index)) {

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
  if (file_name.endsWith(".lua")) {
    return "icon_script.png";
  }

  // Generic file icon.
  return "icon_file.png";
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

  if (source_model->index(source_model->rootPath()).parent() == source_parent) {
    // This is a top-level item: only keep the quest data directory.
    if (is_quest_data_index(source_index)) {
      return true;
    }
    return false;
  }

  if (source_model->isDir(source_index)) {
    // Keep all other directories.
    return true;
  }

  QString file_name = source_model->fileName(source_index);
  QString file_path = source_model->filePath(source_index);

  QString lua_extension = ".lua";
  if (file_name.endsWith(lua_extension)) {
    // Keep all .lua scripts except map scripts.
    QString file_path_dat = file_path.replace(file_path.lastIndexOf(lua_extension), lua_extension.size(), ".dat");
    Solarus::ResourceType resource_type;
    QString element_id;
    if (quest.is_resource_element(file_path_dat, resource_type, element_id) &&
        resource_type == Solarus::ResourceType::MAP) {
      return false;
    }

    return true;
  }

  // Keep resources.
  Solarus::ResourceType resource_type;
  QString element_id;
  if (quest.is_resource_element(file_path, resource_type, element_id)) {
    return true;
  }

  return false;
}

/**
 * @brief Returns whether a source index is the index of the quest data directory.
 * @param source_index An index in the source model.
 * @return \c true if this is the quest data directory.
 */
bool QuestFilesModel::is_quest_data_index(const QModelIndex& source_index) const {

  return source_model->filePath(source_index) == quest.get_data_path();
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

  QModelIndex source_index = mapToSource(index);
  QModelIndex file_source_index = source_model->index(
        source_index.row(), FILE_COLUMN, source_index.parent());
  return source_model->filePath(file_source_index);
}
