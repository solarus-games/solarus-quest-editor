/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
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
#include "widgets/border_set_tree_view.h"
#include "border_set_model.h"
#include "tileset_model.h"

namespace SolarusEditor {

/**
 * @brief Creates an empty border set tree view.
 * @param parent The parent object or nullptr.
 */
BorderSetTreeView::BorderSetTreeView(QWidget* parent) :
  QTreeView(parent),
  tileset(nullptr),
  model(nullptr) {

  setIconSize(QSize(32, 32));
  setSelectionBehavior(SelectItems);
  setHeaderHidden(true);

  setDragDropMode(DragDrop);
  setDragEnabled(true);
  setAcceptDrops(true);  
}

/**
 * @brief Sets the tileset to represent in this view.
 * @param tileset The tileset model.
 */
void BorderSetTreeView::set_tileset(TilesetModel& tileset) {

  this->tileset = &tileset;

  model = new BorderSetModel(tileset);
  setModel(model);
  if (tileset.get_num_border_sets() > 0) {
    resizeColumnToContents(0);
  }

  connect(model, SIGNAL(change_border_set_patterns_requested(QString, QStringList)),
          this, SIGNAL(change_border_set_patterns_requested(QString, QStringList)));

  connect(&tileset, &TilesetModel::border_set_created,
          [this](const QString& border_set_id) {
    QModelIndex index = this->model->get_border_set_index(border_set_id);
    expand(index);
    resizeColumnToContents(0);
    selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    scrollTo(index, ScrollHint::PositionAtTop);
  });

  connect(&tileset, &TilesetModel::border_set_deleted,
          [this](const QString&) {
    resizeColumnToContents(0);
  });

  connect(&tileset, &TilesetModel::border_set_id_changed,
          [this](const QString&, const QString& new_id) {
    set_selected_border_set_id(new_id);
  });
}

/**
 * @brief Returns the currently selected border set id.
 * @return The selected border set id or an empty string.
 */
QString BorderSetTreeView::get_selected_border_set_id() const {

  const QModelIndexList& selected_indexes = selectionModel()->selectedIndexes();
  for (const QModelIndex& index : selected_indexes) {
    if (model->is_border_set_index(index) ||
        model->is_pattern_index(index)
    ) {
      return model->get_border_set_id(index);
    }
  }
  return QString();
}

/**
 * @brief Selects a border set id.
 * @param border_set_id A border set id, or an empty string to select nothing.
 */
void BorderSetTreeView::set_selected_border_set_id(const QString& border_set_id) {

  if (border_set_id.isEmpty()) {
    selectionModel()->clear();
    return;
  }

  const QModelIndex& index = model->get_border_set_index(border_set_id);
  if (index.isValid()) {
    selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
  }
}

/**
 * @brief Deletes the selected border sets or patterns.
 */
void BorderSetTreeView::delete_border_set_selection_requested() {

  const QModelIndexList& selected_indexes = selectionModel()->selectedIndexes();
  QStringList border_sets_to_delete;
  QList<QPair<QString, BorderKind>> patterns_to_delete;
  for (const QModelIndex& index : selected_indexes) {
    const QString& border_set_id = model->get_border_set_id(index);
    if (model->is_border_set_index(index)) {
      // Delete a full border set.
      border_sets_to_delete << border_set_id;
    }
    else if (model->is_pattern_index(index)) {
      // Delete a pattern.
      if (!border_sets_to_delete.contains(border_set_id)) {
        patterns_to_delete << qMakePair(border_set_id, model->get_border_kind(index));
      }
    }
  }

  if (!border_sets_to_delete.isEmpty()) {
    emit delete_border_sets_requested(border_sets_to_delete);
  }

  if (!patterns_to_delete.isEmpty()) {
    emit delete_border_set_patterns_requested(patterns_to_delete);
  }
}

}
