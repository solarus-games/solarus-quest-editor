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
#include "gui/zoom_tool.h"
#include "view_settings.h"
#include <QScrollBar>

/**
 * @brief Creates a map view.
 * @param parent The parent widget or nullptr.
 */
MapView::MapView(QWidget* parent) :
  QGraphicsView(parent),
  scene(nullptr),
  view_settings(nullptr),
  zoom(1.0) {

  setAlignment(Qt::AlignTop | Qt::AlignLeft);

  ViewSettings* view_settings = new ViewSettings(this);
  set_view_settings(*view_settings);
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
    scale(MapScene::scene_to_quest(1.0), MapScene::scene_to_quest(1.0));

    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);

    // Install panning and zooming helpers.
    new PanTool(this);
    new ZoomTool(this);
  }
}

/**
 * @brief Sets the view settings for this map view.
 *
 * When they change, the map view is updated accordingly.
 *
 * @param view_settings The settings to watch.
 */
void MapView::set_view_settings(ViewSettings& view_settings) {

  this->view_settings = &view_settings;

  connect(this->view_settings, SIGNAL(zoom_changed(double)),
          this, SLOT(update_zoom()));
  view_settings.set_zoom(2.0);  // Initial zoom: x2.

  horizontalScrollBar()->setValue(0);
  verticalScrollBar()->setValue(0);
}

/**
 * @brief Sets the zoom level of the view from the settings.
 *
 * Zooming will be anchored at the mouse position.
 * The zoom value will be clamped between 0.25 and 4.0.
 */
void MapView::update_zoom() {

  if (view_settings == nullptr) {
    return;
  }

  double zoom = view_settings->get_zoom();
  zoom = qMin(4.0, qMax(0.25, zoom));

  if (zoom == this->zoom) {
    return;
  }

  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  double scale_factor = zoom / this->zoom;
  scale(scale_factor, scale_factor);
  this->zoom = zoom;
}

/**
 * @brief Scales the view by a factor of 2.
 *
 * Zooming will be anchored at the mouse position.
 * The maximum zoom value is 4.0: this function does nothing if you try to
 * zoom more.
 */
void MapView::zoom_in() {

  if (view_settings == nullptr) {
    return;
  }

  view_settings->set_zoom(view_settings->get_zoom() * 2.0);
}

/**
 * @brief Scales the view by a factor of 0.5.
 *
 * Zooming will be anchored at the mouse position.
 * The maximum zoom value is 0.25: this function does nothing if you try to
 * zoom less.
 */
void MapView::zoom_out() {

  if (view_settings == nullptr) {
    return;
  }

  view_settings->set_zoom(view_settings->get_zoom() / 2.0);
}

/**
 * @brief Shows or hides entities on a layer.
 * @param layer The layer to change.
 * @param visible @c true to show the layer, @c false to hide it.
 */
void MapView::set_layer_visible(Layer layer, bool visible) {

  if (scene == nullptr) {
    return;
  }

  scene->set_layer_visible(layer, visible);
}

/**
 * @brief Shows or hides entities of the specified type.
 * @param type The entity type to change.
 * @param visible @c true to show entities of that type, @c false to hide them.
 */
void MapView::set_entity_type_visible(EntityType type, bool visible) {

  scene->set_entity_type_visible(type, visible);
}
