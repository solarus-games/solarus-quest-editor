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
#include "gui/tileset_model.h"
#include "gui/tileset_scene.h"
#include "gui/tileset_view.h"
#include <QScrollBar>

/**
 * @brief Creates a tileset view.
 * @param parent The parent widget or nullptr.
 */
TilesetView::TilesetView(QWidget* parent) :
  QGraphicsView(parent) {

  setAlignment(Qt::AlignTop | Qt::AlignLeft);
  setDragMode(QGraphicsView::RubberBandDrag);
  setBackgroundBrush(palette().window());
}

/**
 * @brief Sets the tileset to represent in this view.
 * @param model The tileset model.
 */
void TilesetView::set_model(TilesetModel& model) {

  this->model = &model;

  // Create the scene from the model.
  setScene(new TilesetScene(model, this));

  if (model.get_patterns_image().isNull()) {
    return;
  }

  // Enable zoom features if there is an image.
  scale(2.0, 2.0);  // Initial zoom: x2.
  horizontalScrollBar()->setValue(0);
  verticalScrollBar()->setValue(0);
}
