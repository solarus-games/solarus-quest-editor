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
  tileset(nullptr) {

  setIconSize(QSize(32, 32));
  setSelectionMode(QAbstractItemView::SingleSelection);
  setHeaderHidden(true);

}

/**
 * @brief Sets the tileset to represent in this view.
 * @param tileset The tileset model.
 */
void BorderSetTreeView::set_tileset(TilesetModel& tileset) {

  this->tileset = &tileset;

  setModel(new BorderSetModel(tileset));
  expandAll();
  resizeColumnToContents(0);
}

}
