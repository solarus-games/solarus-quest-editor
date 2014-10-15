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
#include "gui/tile_patterns_list_view.h"
#include "gui/tileset_model.h"

/**
 * @brief Creates an empty tile patterns list view.
 * @param parent The parent object or nullptr.
 */
TilePatternsListView::TilePatternsListView(QWidget* parent) :
  QListView(parent) {

  setIconSize(QSize(32, 32));
  setUniformItemSizes(true);
  setViewMode(IconMode);
  setResizeMode(Adjust);
  setSpacing(10);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
}

/**
 * @brief Sets the tileset to represent in this view.
 * @param model The tileset model.
 */
void TilePatternsListView::set_model(TilesetModel& model) {

  QListView::setModel(&model);
  selectionModel()->deleteLater();
  setSelectionModel(&model.get_selection());
}
