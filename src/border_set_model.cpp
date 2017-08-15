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
#include "border_set_model.h"
#include "tileset_model.h"

namespace SolarusEditor {

/**
 * @brief Creates a border set model for the given tileset.
 * @param tileset The tileset.
 * @param parent The parent object or nullptr.
 */
BorderSetModel::BorderSetModel(TilesetModel& tileset, QObject* parent) :
  QAbstractItemModel(parent),
  tileset(tileset) {

}

/**
 * @brief Returns the number of columns in the model.
 * @param parent Parent index.
 * @return The number of columns
 */
int BorderSetModel::columnCount(const QModelIndex& parent) const {
  Q_UNUSED(parent);
  return 1;
}

/**
 * @brief Returns the number of rows under a node.
 * @param parent Parent index.
 * @return The number of rows.
 */
int BorderSetModel::rowCount(const QModelIndex& parent) const {

  if (!parent.isValid()) {
    // Root item.
    int num_rows = tileset.get_num_border_sets();
    return num_rows;
  }

  // TODO
  return 0;
}

/**
 * @brief Returns the index of an item.
 * @param row Row of the item.
 * @param column Column of the item.
 * @param parent Parent of the item.
 * @return The corresponding index. Returns an invalid index if there is no
 * such item.
 */
QModelIndex BorderSetModel::index(int row, int column, const QModelIndex& parent) const {

  Q_UNUSED(parent);
  if (!parent.isValid()) {
    // Root item.
    return createIndex(row, column);
  }

  // TODO
  return QModelIndex();
}

/**
 * @brief Returns the parent of the model item with the given index.
 * @param index Index to get the parent of.
 * @return The parent index, or an invalid index if the item has no parent.
 */
QModelIndex BorderSetModel::parent(const QModelIndex& model_index) const {

  Q_UNUSED(model_index);
  // TODO
  return QModelIndex();
}

/**
 * @brief Returns the data of an item for a given role.
 * @param index Index of the item to get.
 * @param role The wanted role.
 * @return The data.
 */
QVariant BorderSetModel::data(const QModelIndex& model_index, int role) const {

  if (!model_index.isValid()) {
    return QVariant();
  }

  int row = model_index.row();

  switch (role) {
      case Qt::DisplayRole:
    return tileset.get_border_set_ids()[row];

  }

  return QVariant();
}

}
