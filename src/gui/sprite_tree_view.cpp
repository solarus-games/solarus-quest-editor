/*
 * Copyright (C) 2014-2015 Christopho, Solarus - http://www.solarus-games.org
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
#include "gui/sprite_tree_view.h"
#include "sprite_model.h"
#include <QAction>

/**
 * @brief Creates an empty sprite tree view.
 * @param parent The parent object or nullptr.
 */
SpriteTreeView::SpriteTreeView(QWidget* parent) :
  QTreeView(parent) {

  setIconSize(QSize(32, 32));
  setSelectionMode(QAbstractItemView::SingleSelection);
  setHeaderHidden(true);
}

/**
 * @brief Sets the sprite to represent in this view.
 * @param model The sprite model.
 */
void SpriteTreeView::set_model(SpriteModel& model) {

  SpriteTreeView::setModel(&model);
  selectionModel()->deleteLater();
  setSelectionModel(&model.get_selection_model());
}


