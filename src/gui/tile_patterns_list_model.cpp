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
#include "gui/tile_patterns_list_model.h"
#include <QIcon>

/**
 * @brief Creates a tiles pattern list model.
 * @param tileset The tileset to represent the patterns from.
 * @param parent The parent object or nullptr.
 */
TilePatternsListModel::TilePatternsListModel(
    TilesetData& tileset, QObject* parent) :
  QAbstractListModel(parent),
  tileset(tileset) {

}

/**
 * @brief Returns the number of items in the list.
 *
 * This is the number of patterns of the tileset.
 *
 * @return The number of patterns.
 */
int TilePatternsListModel::rowCount(const QModelIndex& /* parent */) const {
  return tileset.get_num_patterns();
}

/**
 * @brief Returns the datat of an item for a given role.
 * @param index Index of the item to get.
 * @param role The wanted role.
 * @return The data.
 */
QVariant TilePatternsListModel::data(const QModelIndex& index, int role) const {

  switch (role) {

  case Qt::DisplayRole:
    // TODO show the id
    return "Pattern " + QString::number(index.row());
    break;

  case Qt::DecorationRole:
    // TODO show the pattern image
    return QIcon(":/images/icon_solarus_200.png");
    break;

  default:
    break;
  }

  return QVariant();
}
