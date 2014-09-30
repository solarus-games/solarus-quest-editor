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
#include "quest_files_model.h"
#include <QFileSystemModel>

/**
 * @brief Creates a quest files model.
 * @param parent Data path of the quest to represent.
 */
QuestFilesModel::QuestFilesModel(QString quest_data_path):
  QSortFilterProxyModel(nullptr),
  quest_data_path(quest_data_path),
  source_model(new QFileSystemModel) {

  source_model->setRootPath(quest_data_path);
  source_model->setReadOnly(false);
  setSourceModel(source_model);

}

/**
 * @brief Returns the index of the root item of the quest.
 * @return The root index.
 */
QModelIndex QuestFilesModel::get_quest_root_index() const {

  return mapFromSource(source_model->index(quest_data_path));
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

  switch (role) {

  case Qt::DisplayRole:
    // Text of each file item.

    switch (index.column()) {

    case 0:  // File.
      return source_model->fileName(source_index);

    case 1:  // Friendly resource name.
      // TODO
      return "";

    case 2:  // Type
      // TODO
      return "";

    }

  case Qt::EditRole:
    // Editable file name.
    if (index.column() == 0) {
      return source_model->fileName(source_index);
    }
    return QVariant();

  case Qt::DecorationRole:
    // Icon.
    if (index.column() == 0) {
      return get_quest_file_icon(source_index);
    }
    return QVariant();  // No icon in other columns.
  }

  // For other roles, rely on the standard settings.
  return QSortFilterProxyModel::data(index, role);
}

/**
 * @brief Returns an appropriate icon for the specified quest file.
 * @param index Index of a file item in the source model.
 * @return An appropriate icon to represent this file.
 */
QIcon QuestFilesModel::get_quest_file_icon(const QModelIndex& source_index) const {

  QString icon_file_name = "icon_file.png";
  if (source_model->isDir(source_index)) {
    icon_file_name = "icon_folder_closed.png";
  }
  return QIcon(":/images/" + icon_file_name);
}
