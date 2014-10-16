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
  QGraphicsView(parent),
  zoom(1.0) {

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
  set_zoom(2.0);  // Initial zoom: x2.
  horizontalScrollBar()->setValue(0);
  verticalScrollBar()->setValue(0);
}

/**
 * @brief Returns the zoom level of the view.
 * @return The zoom level.
 */
double TilesetView::get_zoom() const {
  return zoom;
}

/**
 * @brief Sets the zoom level of the view.
 *
 * Zooming will be anchored at the mouse position.
 * The zoom value will be clamped between 0.25 and 4.0.
 *
 * @param zoom The zoom to set.
 */
void TilesetView::set_zoom(double zoom) {

  zoom = qMin(4.0, qMax(0.25, zoom));

  if (zoom == this->zoom) {
    return;
  }

  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  scale(1.0 / this->zoom, 1.0 / this->zoom);
  this->zoom = zoom;
  scale(zoom, zoom);
}

/**
 * @brief Scales the view by a factor of 2.
 *
 * Zooming will be anchored at the mouse position.
 * The maximum zoom value is 4.0: this function does nothing if you try to
 * zoom more.
 */
void TilesetView::zoom_in() {

  set_zoom(get_zoom() * 2.0);
}

/**
 * @brief Scales the view by a factor of 0.5.
 *
 * Zooming will be anchored at the mouse position.
 * The maximum zoom value is 0.25: this function does nothing if you try to
 * zoom less.
 */
void TilesetView::zoom_out() {

  set_zoom(get_zoom() / 2.0);
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

/**
 * @brief Receives a mouse wheel event.
 * @param event The event to handle.
 */
void TilesetView::wheelEvent(QWheelEvent* event) {

  if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
    // Control + wheel: zoom in or out.
    if (event->delta() > 0) {
      zoom_in();
    }
    else {
      zoom_out();
    }
    return;  // Don't forward the event to the scrollbars.
  }

  QGraphicsView::wheelEvent(event);
}
