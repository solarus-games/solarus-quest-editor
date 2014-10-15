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
#include <QApplication>
#include <QGraphicsItem>
#include <QMouseEvent>
#include <QScrollBar>

/**
 * @brief Creates a tileset view.
 * @param parent The parent widget or nullptr.
 */
TilesetView::TilesetView(QWidget* parent) :
  QGraphicsView(parent) {

  setAlignment(Qt::AlignTop | Qt::AlignLeft);
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

  // Enable useful features if there is an image.
  setDragMode(QGraphicsView::RubberBandDrag);
  scale(2.0, 2.0);  // Initial zoom: x2.
  horizontalScrollBar()->setValue(0);
  verticalScrollBar()->setValue(0);
}

/**
 * @brief Receives a mouse press event.
 *
 * Reimplemented to scroll the view when the middle mouse button is pressed.
 *
 * @param event The event to handle.
 */
void TilesetView::mousePressEvent(QMouseEvent* event) {

  if (event->button() == Qt::MidButton) {
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    pan_initial_point = QPoint(
          horizontalScrollBar()->value() + event->x(),
          verticalScrollBar()->value() + event->y()
          );
    return;
  }

  QGraphicsView::mousePressEvent(event);
}

/**
 * @brief Receives a mouse release event.
 *
 * Reimplemented to scroll the view when the middle mouse button is pressed.
 *
 * @param event The event to handle.
 */
void TilesetView::mouseReleaseEvent(QMouseEvent* event) {

  if (event->button() == Qt::MidButton) {
    QApplication::restoreOverrideCursor();
    return;
  }

  QGraphicsView::mouseReleaseEvent(event);
}

/**
 * @brief Receives a mouse move event.
 *
 * Reimplemented to scroll the view when the middle mouse button is pressed.
 *
 * @param event The event to handle.
 */
void TilesetView::mouseMoveEvent(QMouseEvent* event) {

  if ((event->buttons() & Qt::MidButton) == Qt::MidButton) {

    QPoint scroll_point(
          pan_initial_point.x() - event->x(),
          pan_initial_point.y() - event->y()
    );
    horizontalScrollBar()->setValue(scroll_point.x());
    verticalScrollBar()->setValue(scroll_point.y());
    return;
  }

  QGraphicsView::mouseMoveEvent(event);
}
