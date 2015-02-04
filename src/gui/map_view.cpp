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
#include "gui/map_scene.h"
#include "gui/map_view.h"
#include "gui/pan_tool.h"
#include <QScrollBar>

/**
 * @brief Creates a map view.
 * @param parent The parent widget or nullptr.
 */
MapView::MapView(QWidget* parent) :
  QGraphicsView(parent),
  scene(nullptr),
  zoom(1.0) {

  setAlignment(Qt::AlignTop | Qt::AlignLeft);
}

/**
 * @brief Sets the map to represent in this view.
 * @param model The map model, or nullptr to remove any model.
 * This class does not take ownership on the model.
 * The model can be deleted safely.
 */
void MapView::set_model(MapModel* model) {

  if (this->model != nullptr) {
    this->model = nullptr;
    this->scene = nullptr;
  }

  this->model = model;

  if (model != nullptr) {
    // Create the scene from the model.
    scene = new MapScene(*model, this);
    setScene(scene);

    // Enable useful features if there is an image.
    setDragMode(QGraphicsView::RubberBandDrag);

    // Scale the view of 0.5 because the whole scene works with upscaled
    // coordinates.
    scale(MapScene::scene_to_quest_factor, MapScene::scene_to_quest_factor);
    set_zoom(2.0);  // Initial zoom.

    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);

    // Pan the view with the middle mouse button.
    new PanTool(this);
  }
}

/**
 * @brief Returns the zoom level of the view.
 * @return The zoom level.
 */
double MapView::get_zoom() const {
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
void MapView::set_zoom(double zoom) {

  zoom = qMin(4.0, qMax(0.25, zoom));

  if (zoom == this->zoom) {
    return;
  }

  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  scale(zoom / this->zoom, zoom / this->zoom);
  this->zoom = zoom;

  emit zoom_changed(zoom);
}

/**
 * @brief Scales the view by a factor of 2.
 *
 * Zooming will be anchored at the mouse position.
 * The maximum zoom value is 4.0: this function does nothing if you try to
 * zoom more.
 */
void MapView::zoom_in() {

  set_zoom(get_zoom() * 2.0);
}

/**
 * @brief Scales the view by a factor of 0.5.
 *
 * Zooming will be anchored at the mouse position.
 * The maximum zoom value is 0.25: this function does nothing if you try to
 * zoom less.
 */
void MapView::zoom_out() {

  set_zoom(get_zoom() / 2.0);
}
