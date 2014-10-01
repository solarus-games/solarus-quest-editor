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
#include <QFileSystemModel>

namespace {

/**
 * Description of each resource type.
 */
const QMap<Solarus::ResourceType, QString> resource_type_friendly_names = {
  { Solarus::ResourceType::MAP,      "Map"           },
  { Solarus::ResourceType::TILESET,  "Tileset"       },
  { Solarus::ResourceType::SPRITE,   "Sprite"        },
  { Solarus::ResourceType::MUSIC,    "Music"         },
  { Solarus::ResourceType::SOUND,    "Sound"         },
  { Solarus::ResourceType::ITEM,     "Item"          },
  { Solarus::ResourceType::ENEMY,    "Enemy"         },
  { Solarus::ResourceType::ENTITY,   "Custom entity" },
  { Solarus::ResourceType::LANGUAGE, "Language"      },
  { Solarus::ResourceType::FONT,     "Font"          },
};


}

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

  // File, Name, Type
  return 3;
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

    case 0:
      return "File";

    case 1:
      return "Description";

    case 2:
      return "Type";
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
  QModelIndex file_source_index = source_model->index(source_index.row(), 0, source_index.parent());  // First column.
  QString file_name = source_model->fileName(file_source_index);
  QString file_path = source_model->filePath(file_source_index);
  Solarus::ResourceType resource_type;

  switch (role) {

  case Qt::DisplayRole:
    // Text of each file item.

    switch (index.column()) {

    case 0:  // File.
      if (is_quest_data_index(source_index)) {
        // Data directory: show the quest name instead of "data".
        return quest.get_name();
      }

      if (quest.is_resource_element(file_path, resource_type)) {
        // A resource element: show its id (remove the extension).
        if (resource_type != Solarus::ResourceType::LANGUAGE) {
          return file_name.section('.', -2, -2);
        }
      }
      return file_name;

    case 1:  // Resource element description.
      // TODO
      return "";

    case 2:  // Type
      if (is_quest_data_index(source_index)) {
        // Quest data directory (top-level item).
        return "Quest";
      }

      if (file_path == quest.get_main_script_path()) {
        // main.lua
        return "Main Lua script";
      }

      if (quest.is_resource_path(file_path, resource_type)) {
        // A resource element folder.
        return resource_type_friendly_names[resource_type] + " folder";
      }

      if (quest.is_resource_element(file_path, resource_type)) {
        // A declared resource element.
        return resource_type_friendly_names[resource_type];
      }

      if (file_name.endsWith(".lua")) {
        // An arbitrary Lua script.
        return "Lua script";
      }

      // Not a file managed by Solarus.
      return "";

    }

  case Qt::EditRole:
    // Editable file name.
    if (index.column() == 0) {
      return file_name;
    }
    // TODO allow to edit the resource element description.
    return QVariant();

  case Qt::DecorationRole:
    // Icon.
    if (index.column() == 0) {
      return QIcon(":/images/" + get_quest_file_icon_name(source_index));
    }
    return QVariant();  // No icon in other columns.
  }

  // For other roles, rely on the standard settings.
  return QSortFilterProxyModel::data(index, role);
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

  // Quest data directory.
  if (is_quest_data_index(source_index)) {
    return "icon_solarus.png";
  }

  // Directory icon.
  if (source_model->isDir(source_index)) {

    if (quest.is_resource_path(file_path, resource_type)) {
      return "icon_folder_open_" +
          QString::fromStdString(Solarus::QuestResourceList::get_resource_type_name(resource_type)) +
          ".png";
    }

    return "icon_folder_open.png";
  }

  // This is not a directory.

  // Resource element.
  if (quest.is_resource_element(file_path, resource_type)) {
    return "icon_resource_" +
        QString::fromStdString(Solarus::QuestResourceList::get_resource_type_name(resource_type)) +
        ".png";
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

  QModelIndex source_index = source_model->index(source_row, 0, source_parent);

  if (source_model->index(source_model->rootPath()).parent() == source_parent) {
    // This is a top-level item: only keep the data directory.
    if (is_quest_data_index(source_index)) {
      return true;
    }
    return false;
  }

  if (source_model->isDir(source_index)) {
    // Keep all directories.
    return true;
  }

  QString file_name = source_model->fileName(source_index);
  QString file_path = source_model->filePath(source_index);

  QString lua_extension = ".lua";
  if (file_name.endsWith(lua_extension)) {
    // Keep all .lua scripts except map scripts.
    QString file_path_dat = file_path.replace(file_path.lastIndexOf(lua_extension), lua_extension.size(), ".dat");
    Solarus::ResourceType resource_type;
    if (quest.is_resource_element(file_path_dat, resource_type) &&
        resource_type == Solarus::ResourceType::MAP) {
      return false;
    }

    return true;
  }

  // Keep resources.
  Solarus::ResourceType resource_type;
  if (quest.is_resource_element(file_path, resource_type)) {
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
