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
#include "border_set_model.h"
#include "tileset_model.h"
#include <QMimeData>

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
    border_set_indexes << BorderSetIndex(border_set_id);
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

  if (is_border_set_index(parent)) {
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
  return createIndex(row, column, border_set_indexes[parent.row()].border_set_id.get());
}

/**
 * @brief Returns the parent of the model item with the given index.
 * @param index Index to get the parent of.
 * @return The parent index, or an invalid index if the item has no parent.
 */
QModelIndex BorderSetModel::parent(const QModelIndex& index) const {

  if (!index.isValid()) {
    // Root item.
    return QModelIndex();
  }

  if (is_border_set_index(index)) {
    // Border set item: parent is root.
    return QModelIndex();
  }

  const QString& border_set_id = get_border_set_id(index);
  // TODO store a QString -> int cache
  for (int row = 0; row < rowCount(); ++row) {
    QModelIndex row_index = this->index(row, 0);
    if (get_border_set_id(row_index) == border_set_id) {
      return row_index;
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
QVariant BorderSetModel::data(const QModelIndex& index, int role) const {

  if (!index.isValid()) {
    return QVariant();
  }

  int row = index.row();
  int column = index.column();

  if (column < 0 || column >= columnCount()) {
    return QVariant();
  }

  const QString border_set_id = get_border_set_id(index);
  const QString pattern_id = get_pattern_id(index);
  BorderKind border_kind = get_border_kind(index);

  if (is_border_set_index(index)) {
    // Border set item.

    if (row < 0 || row >= tileset.get_num_border_sets()) {
      return QVariant();
    }

    switch (role) {

    case Qt::DisplayRole:

      if (column == 0) {
        return border_set_id;
      }
      break;

    }

  }
  else if (is_pattern_index(index)) {
    // Pattern item.

    if (row < 0 || row >= 12) {
      return QVariant();
    }

    if (column == 0) {
      switch (role) {

      case Qt::DecorationRole:
        // Icon representing the border kind.
        return QIcon(QString(":/images/border_kind_%1.png").arg(row));

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
 * @brief Returns the flags of the given index.
 * @param index A model index.
 * @return The corresponding flags.
 */
Qt::ItemFlags BorderSetModel::flags(const QModelIndex& index) const {

  Qt::ItemFlags flags = QAbstractItemModel::flags(index);

  flags = flags | Qt::ItemIsDropEnabled;
  if (index.column() == 1) {
    flags = flags | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
  }
  else {
    flags = flags & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsSelectable;
  }

  return flags;
}

/**
 * @brief Returns the MIME types supported by drag and drop operations.
 * @return The plain text mime type.
 */
QStringList BorderSetModel::mimeTypes() const {
  return QStringList() << "text/plain";
}

/**
 * @brief Returns serialized data corresponding to the given indexes.
 * @param indexes A list of indexes.
 * @return The corresponding MIME data.
 */
QMimeData* BorderSetModel::mimeData(const QModelIndexList& indexes) const {

  if (indexes.isEmpty()) {
    return nullptr;
  }

  QStringList lines;
  Q_FOREACH(const QModelIndex& index, indexes) {
    const QString& pattern_id = get_pattern_id(index);
    if (!pattern_id.isEmpty()) {
      lines << pattern_id;
    }
  }

  QMimeData* data = new QMimeData();
  data->setText(lines.join("\n"));
  return data;
}

/**
 * @brief Returns the drop actions supported by this model.
 * @return The drop actions.
 */
Qt::DropActions BorderSetModel::supportedDropActions() const {

  return Qt::CopyAction | Qt::MoveAction;
}


/**
 * @brief Handles dropping serialized data at the given place.
 * @param data The data to drop.
 * @param action The drop action.
 * @param row Row just after the drop.
 * @param column Column of the drop.
 * @param parent Parent index where the drop occurs.
 * @return @c true if the drop is possible there.
 */
bool BorderSetModel::canDropMimeData(
    const QMimeData* data,
    Qt::DropAction action,
    int row,
    int column,
    const QModelIndex& parent
) const {

  Q_UNUSED(action);
  Q_UNUSED(row);
  Q_UNUSED(column);
  Q_UNUSED(parent);

  return data->hasText();
}

/**
 * @brief Handles dropping serialized data at the given place.
 * @param data The data dropped.
 * @param action The drop action.
 * @param row Row just after the drop.
 * @param column Column of the drop.
 * @param parent Parent index where the drop occurs.
 * @return @c true if the drop was handled.
 */
bool BorderSetModel::dropMimeData(
    const QMimeData* data,
    Qt::DropAction action,
    int row,
    int column,
    const QModelIndex& parent
) {
  Q_UNUSED(action);
  Q_UNUSED(row);
  Q_UNUSED(column);

  if (!data->hasText()) {
    return false;
  }

  QString text = data->text();
  QStringList pattern_ids = text.split("\n");

  if (pattern_ids.isEmpty()) {
    return false;
  }

  const QString& pattern_id = pattern_ids.first();  // TODO support multiple patterns.

  if (is_pattern_index(parent)) {
    // Drop onto a pattern item.
    const QString& border_set_id = get_border_set_id(parent);
    BorderKind border_kind = get_border_kind(parent);
    tileset.set_border_set_pattern(border_set_id, border_kind, pattern_id);
  }

  return false;
}

/**
 * @brief Returns whether a model index corresponds to a border set item.
 * @param index A model index.
 * @return @c true if this is a border set index.
 */
bool BorderSetModel::is_border_set_index(const QModelIndex& index) const {

  if (!index.isValid()) {
    // Root item.
    return false;
  }

  void* internal_pointer = index.internalPointer();
  if (internal_pointer == nullptr) {
    // Border set index.
    return true;
  }

  // Pattern index.
  return false;
}

/**
 * @brief Returns whether a model index corresponds to a pattern item.
 * @param index A model index.
 * @return @c true if this is a pattern index.
 */
bool BorderSetModel::is_pattern_index(const QModelIndex& index) const {

  if (!index.isValid()) {
    // Root item.
    return false;
  }

  void* internal_pointer = index.internalPointer();
  if (internal_pointer == nullptr) {
    // Border set index.
    return false;
  }

  // Pattern index.
  return true;
}

/**
 * @brief Returns the border set id of the given index.
 * @param index An item index.
 * @return The corresponding border set id if this is a border set item
 * or a pattern item, or an empty string otherwise.
 */
QString BorderSetModel::get_border_set_id(const QModelIndex& index) const {

  if (is_border_set_index(index)) {

    int row = index.row();
    if (row < 0 || row >= tileset.get_num_border_sets()) {
      return QString();
    }

    return *border_set_indexes[row].border_set_id.get();
  }

  if (is_pattern_index(index)) {
    void* internal_pointer = index.internalPointer();
    return *static_cast<QString*>(internal_pointer);
  }

  return QString();
}

/**
 * @brief Returns the pattern id of the given index.
 * @param index An item index.
 * @return The corresponding pattern id if this is a pattern item,
 * or an empty string otherwise.
 */
QString BorderSetModel::get_pattern_id(const QModelIndex& index) const {

  if (!is_pattern_index(index)) {
    return QString();
  }

  if (index.row() < 0 || index.row() >= 12) {
    return QString();
  }

  BorderKind border_kind = get_border_kind(index);
  const QString& border_set_id = get_border_set_id(index);
  return tileset.get_border_set_pattern(border_set_id, border_kind);
}

/**
 * @brief Returns the border kind of the pattern at the given index.
 * @param index An item index.
 * @return The corresponding border kind if this is a pattern item,
 * or @c BorderKind::NONE otherwise.
 */
BorderKind BorderSetModel::get_border_kind(const QModelIndex& index) const {

  if (!is_pattern_index(index)) {
    return BorderKind::NONE;
  }

  return static_cast<BorderKind>(index.row());
}

/**
 * @brief Returns the border set id and pattern id of the given index.
 * @param index An item index.
 * @return The corresponding border set id and pattern id if any.
 */
QPair<QString, QString> BorderSetModel::get_pattern_info(const QModelIndex& index) const {

  return qMakePair(get_border_set_id(index), get_pattern_id(index));
}

}
