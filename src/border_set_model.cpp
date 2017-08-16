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
#include "border_kind_traits.h"
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

  Q_FOREACH(const QString& border_set_id, tileset.get_border_set_ids()) {
    border_set_ids << BorderSetId(border_set_id);
  }
}

/**
 * @brief Returns the number of columns in the model.
 * @param parent Parent index.
 * @return The number of columns
 */
int BorderSetModel::columnCount(const QModelIndex& parent) const {
  Q_UNUSED(parent);
  return 2;
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

  const QModelIndex& grand_parent = parent.parent();
  if (!grand_parent.isValid()) {
    // Border set item.
    return 12;
  }

  // Pattern item.
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
    // Root item: create a border set index.
    return createIndex(row, column);
  }

  // Pattern item.
  return createIndex(row, column, border_set_ids[parent.row()].id.get());
}

/**
 * @brief Returns the parent of the model item with the given index.
 * @param index Index to get the parent of.
 * @return The parent index, or an invalid index if the item has no parent.
 */
QModelIndex BorderSetModel::parent(const QModelIndex& model_index) const {

  if (!model_index.isValid()) {
    // Root item.
    return QModelIndex();
  }

  void* internal_pointer = model_index.internalPointer();
  if (internal_pointer == nullptr) {
    // Border set item: parent is root.
    return QModelIndex();
  }

  const QString& border_set_id = *static_cast<QString*>(internal_pointer);
  // TODO store a QString -> int cache
  for (int row = 0; row < rowCount(); ++row) {
    if (get_border_set_id(row) == border_set_id) {
      return index(row, 0);
    }
  }
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
  int column = model_index.column();

  if (column < 0 || column >= columnCount()) {
    return QVariant();
  }

  const QModelIndex& parent = model_index.parent();
  if (!parent.isValid()) {
    // Border set item.

    if (row < 0 || row >= tileset.get_num_border_sets()) {
      return QVariant();
    }
    const QString& border_set_id = get_border_set_id(row);

    switch (role) {

    case Qt::DisplayRole:

      if (column == 0) {
        return border_set_id;
      }
      break;

    }

  }
  else {
    // Pattern item.

    if (row < 0 || row >= 12) {
      return QVariant();
    }

    const QModelIndex& border_set_item = model_index.parent();
    const QString& border_set_id = get_border_set_id(border_set_item.row());
    BorderKind border_kind = static_cast<BorderKind>(row);
    const QString& pattern_id = tileset.get_border_set_pattern(border_set_id, border_kind);

    if (column == 0) {
      switch (role) {

      case Qt::DisplayRole:
        // Name of the border kind.
        // TODO *only* show the icon instead.
        return BorderKindTraits::get_friendly_name(border_kind);

      case Qt::ToolTipRole:
        // Name of the border kind.
        return BorderKindTraits::get_friendly_name(border_kind);
      }
    }
    else if (column == 1) {

      switch (role) {

      case Qt::DecorationRole:
        // Pattern icon.
        return tileset.get_pattern_icon(tileset.id_to_index(pattern_id));

      case Qt::DisplayRole:
        // Name of the pattern.
        return pattern_id;
      }
    }
  }

  return QVariant();
}

/**
 * @brief Returns the border set id of the given row.
 * @param row A row from the root item.
 * @return The corresponding border set id.
 */
QString BorderSetModel::get_border_set_id(int row) const {

  Q_ASSERT(row >= 0 && row < border_set_ids.size());
  return *border_set_ids[row].id.get();
}

}
