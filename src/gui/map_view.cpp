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
#include "gui/gui_tools.h"
#include "gui/map_scene.h"
#include "gui/map_view.h"
#include "gui/mouse_coordinates_tracking_tool.h"
#include "gui/pan_tool.h"
#include "gui/zoom_tool.h"
#include "point.h"
#include "rectangle.h"
#include "view_settings.h"
#include <QGraphicsItem>
#include <QMouseEvent>
#include <QScrollBar>

namespace {

/**
 * @brief State of the map view corresponding to the user doing nothing special.
 *
 * He can select or unselect entities.
 */
class DoingNothingState : public MapView::State {

public:
  DoingNothingState(MapView& view);

  bool mouse_pressed(const QMouseEvent& event) override;
};

/**
 * @brief State of the map view of drawing a selection rectangle.
 */
class DrawingRectangleState : public MapView::State {

public:
  DrawingRectangleState(MapView& view, const QPoint& initial_point);

  void start() override;
  void stop() override;

  bool mouse_moved(const QMouseEvent& event) override;
  bool mouse_released(const QMouseEvent& event) override;

private:
  QPoint initial_point;                     /**< Point where the drawing started, in scene coordinates. */
  QPoint current_point;                     /**< Point where the dragging currently is, in scene coordinates. */
  QGraphicsRectItem* current_area_item;     /**< Graphic item of the rectangle the user is drawing
                                             * (belongs to the scene). */
  QList<QGraphicsItem*> initial_selection;  /**< Items that were selected before the drawing started. */
};

/**
 * @brief State of the map view of moving the selected entities.
 */
class MovingEntitiesState : public MapView::State {

public:
  MovingEntitiesState(MapView& view, const QPoint& initial_point);

  bool mouse_moved(const QMouseEvent& event) override;
  bool mouse_released(const QMouseEvent& event) override;

private:
  QPoint initial_point;      /**< Point where the dragging started, in scene coordinates. */
  QPoint last_point;         /**< Point where the mouse was last time it moved, in scene coordinates. */
  QPoint total_translation;  /**< Total translation from the initial point. */
};

}  // Anonymous namespace.

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

  // Necessary because we draw a custom background.
  setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}

/**
 * @brief Returns the map represented in this view.
 * @return The map model or nullptr if none was set.
 */
MapModel* MapView::get_model() {
  return model.data();
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
    if (view_settings != nullptr) {
      view_settings->set_zoom(2.0);  // Initial zoom: x2.
    }
    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);

    // Install panning and zooming helpers.
    new PanTool(this);
    new ZoomTool(this);
    new MouseCoordinatesTrackingTool(this);

    // Start the state mechanism.
    start_state_doing_nothing();
  }
}

/**
 * @brief Returns the map scene represented in this view.
 * @return The scene or nullptr if no map model was set.
 */
MapScene* MapView::get_scene() {
  return scene;
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
  update_zoom();

  connect(this->view_settings, SIGNAL(grid_visibility_changed(bool)),
          this, SLOT(update_grid_visibility()));
  update_grid_visibility();

  connect(this->view_settings, SIGNAL(layer_visibility_changed(Layer, bool)),
          this, SLOT(update_layer_visibility(Layer)));

  connect(this->view_settings, SIGNAL(entity_type_visibility_changed(EntityType, bool)),
          this, SLOT(update_entity_type_visibility(EntityType)));

  horizontalScrollBar()->setValue(0);
  verticalScrollBar()->setValue(0);
}

/**
 * @brief Changes the state of the view.
 *
 * The previous state if any is destroyed.
 *
 * @param state The new state.
 */
void MapView::set_state(std::unique_ptr<State> state) {

  if (this->state != nullptr) {
    this->state->stop();
  }

  this->state = std::move(state);

  if (this->state != nullptr) {
    this->state->start();
  }
}

/**
 * @brief Moves to the normal state of the map view.
 */
void MapView::start_state_doing_nothing() {

  set_state(std::unique_ptr<State>(new DoingNothingState(*this)));
}

/**
 * @brief Moves to the state of moving the selected entities.
 * @param initial_point Where the user starts dragging the entities,
 * in view coordinates.
 */
void MapView::start_state_moving_entities(const QPoint& initial_point) {

  set_state(std::unique_ptr<State>(new MovingEntitiesState(*this, initial_point)));
}

/**
 * @brief Moves to the state of drawing a rectangle for a selection.
 * @param initial_point Where the user starts drawing the rectangle,
 * in view coordinates.
 */
void MapView::start_state_drawing_rectangle(const QPoint& initial_point) {

  set_state(std::unique_ptr<State>(new DrawingRectangleState(*this, initial_point)));
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
 * @brief Slot called when the mouse coordinates on the view have changed.
 *
 * Translates the coordinates relative to the view into coordinates relative
 * to the map and emits mouse_map_coordinates_changed().
 *
 * @param xy The mouse coordinates relative to the widget.
 */
void MapView::mouse_coordinates_changed(const QPoint& xy) {

  QPoint map_xy = mapToScene(xy).toPoint() - MapScene::get_margin_top_left();
  emit mouse_map_coordinates_changed(map_xy);
}

/**
 * @brief Shows or hides the grid according to the view settings.
 */
void MapView::update_grid_visibility() {

  if (view_settings == nullptr) {
    return;
  }

  if (scene != nullptr) {
    // The foreground has changed.
    scene->invalidate();
  }
}

/**
 * @brief Shows or hides entities on a layer according to the view settings.
 * @param layer The layer to update.
 */
void MapView::update_layer_visibility(Layer layer) {

  if (scene == nullptr) {
    return;
  }

  scene->update_layer_visibility(layer, *view_settings);
}

/**
 * @brief Shows or hides entities of a type according to the view settings.
 * @param type The entity type to update.
 */
void MapView::update_entity_type_visibility(EntityType type) {

  scene->update_entity_type_visibility(type, *view_settings);
}

/**
 * @brief Draws the foreground of the map.
 * @param painter The painter to draw.
 * @param rect The exposed rectangle.
 */
void MapView::drawForeground(QPainter* painter, const QRectF& rectangle) {

  if (view_settings == nullptr || !view_settings->is_grid_visible()) {
    return;
  }

  const int square_size = 16;
  GuiTools::draw_grid(*painter, rectangle.toRect(), square_size);

  QGraphicsView::drawForeground(painter, rectangle);
}

/**
 * @brief Receives a mouse press event.
 * @param event The event to handle.
 */
void MapView::mousePressEvent(QMouseEvent* event) {

  if (model != nullptr && get_scene() != nullptr) {
    if (state->mouse_pressed(*event)) {
      return;
    }
  }

  QGraphicsView::mousePressEvent(event);
}

/**
 * @brief Receives a mouse release event.
 * @param event The event to handle.
 */
void MapView::mouseReleaseEvent(QMouseEvent* event) {

  if (model != nullptr && get_scene() != nullptr) {
    if (state->mouse_released(*event)) {
      return;
    }
  }

  QGraphicsView::mouseReleaseEvent(event);
}

/**
 * @brief Receives a mouse move event.
 * @param event The event to handle.
 */
void MapView::mouseMoveEvent(QMouseEvent* event) {

  if (model != nullptr && get_scene() != nullptr) {
    if (state->mouse_moved(*event)) {
      return;
    }
  }

  QGraphicsView::mouseMoveEvent(event);
}

/**
 * @brief Receives a context menu event.
 * @param event The event to handle.
 */
void MapView::contextMenuEvent(QContextMenuEvent* event) {

  if (model != nullptr && get_scene() != nullptr) {
    if (state->context_menu_requested(*event)) {
      return;
    }
  }

  QGraphicsView::contextMenuEvent(event);
}

/**
 * @brief Returns the indexes of selected entities.
 * @return The selected entities.
 */
QList<EntityIndex> MapView::get_selected_entities() {

  QList<EntityIndex> result;
  if (scene == nullptr) {
    return result;
  }

  for (QGraphicsItem* item : scene->selectedItems()) {
    EntityIndex index = scene->get_item_index(*item);
    if (index.is_valid()) {
      result.append(index);
    }
  }
  return result;
}

/**
 * @brief Requests to move the selected entities with the specified translation.
 * @param translation XY coordinates to add.
 */
void MapView::move_selected_entities(const QPoint& translation) {

  emit move_entities_requested(get_selected_entities(), translation);
}

/**
 * @brief Creates a state.
 * @param view The map view to manage.
 */
MapView::State::State(MapView& view) :
  view(view) {

}

/**
 * @brief Destructor.
 */
MapView::State::~State() {

}

/**
 * @brief Returns the map view managed by this state.
 * @return The map view.
 */
MapView& MapView::State::get_view() {
  return view;
}

/**
 * @brief Returns the map scene managed by this state.
 * @return The map scene.
 */
MapScene& MapView::State::get_scene() {

  return *view.get_scene();
}

/**
 * @brief Returns the map model represented in the view.
 * @return The map model.
 */
MapModel& MapView::State::get_map() {
  return *view.get_model();
}

/**
 * @brief Called when entering this state.
 *
 * Subclasses can reimplement this function to initialize data.
 */
void MapView::State::start() {
}

/**
 * @brief Called when leaving this state.
 *
 * Subclasses can reimplement this function to clean data.
 */
void MapView::State::stop() {
}

/**
 * @brief Called when the mouse is pressed in the map view during this state.
 *
 * Subclasses can reimplement this function to define what happens.
 *
 * @param event The event to handle.
 * @return @c true if the event was handled, @c false to propagate it further.
 */
bool MapView::State::mouse_pressed(const QMouseEvent& event) {

  Q_UNUSED(event);
  return false;
}

/**
 * @brief Called when the mouse is released in the map view during this state.
 *
 * Subclasses can reimplement this function to define what happens.
 *
 * @param event The event to handle.
 * @return @c true if the event was handled, @c false to propagate it further.
 */
bool MapView::State::mouse_released(const QMouseEvent& event) {

  Q_UNUSED(event);
  return false;
}

/**
 * @brief Called when the mouse is moved in the map view during this state.
 *
 * Subclasses can reimplement this function to define what happens.
 *
 * @param event The event to handle.
 * @return @c true if the event was handled, @c false to propagate it further.
 */
bool MapView::State::mouse_moved(const QMouseEvent& event) {

  Q_UNUSED(event);
  return false;
}

/**
 * @brief Called when a context menu is requested in the map view during this
 * state.
 *
 * Subclasses can reimplement this function to define what happens.
 *
 * @param event The event to handle.
 * @return @c true if the event was handled, @c false to propagate it further.
 */
bool MapView::State::context_menu_requested(const QContextMenuEvent& event) {

  Q_UNUSED(event);
  return false;
}

/**
 * @brief Constructor.
 * @param view The map view to manage.
 */
DoingNothingState::DoingNothingState(MapView& view) :
  MapView::State(view) {

}

/**
 * @copydoc MapView::State::mouse_pressed
 */
bool DoingNothingState::mouse_pressed(const QMouseEvent& event) {

  if (event.button() != Qt::LeftButton && event.button() != Qt::RightButton) {
    return true;
  }

  MapView& view = get_view();
  MapScene& scene = get_scene();

  // Left or right button: possibly change the selection.
  QList<QGraphicsItem*> items_under_mouse = view.items(
        QRect(event.pos(), QSize(1, 1)),
        Qt::IntersectsItemBoundingRect  // Pick transparent items too.
        );
  QGraphicsItem* item = items_under_mouse.empty() ? nullptr : items_under_mouse.first();

  const bool control_or_shift = (event.modifiers() & (Qt::ControlModifier | Qt::ShiftModifier));

  bool keep_selected = false;
  if (control_or_shift) {
    // If ctrl or shift is pressed, keep the existing selection.
    keep_selected = true;
  }
  else if (item != nullptr && item->isSelected()) {
    // When clicking an already selected item, keep the existing selection too.
    keep_selected = true;
  }

  if (!keep_selected) {
    scene.clearSelection();
  }

  if (event.button() == Qt::LeftButton) {

    if (item != nullptr) {

      if (control_or_shift) {
        // Left-clicking an item while pressing control or shift: toggle it.
        item->setSelected(!item->isSelected());
      }
      else {
        if (!item->isSelected()) {
          // Select the item.
          item->setSelected(true);
        }
        // Allow to move selected items.
        view.start_state_moving_entities(event.pos());
      }
    }
    else {
      // Left click outside items: trace a selection rectangle.
      view.start_state_drawing_rectangle(event.pos());
    }
  }

  else if (event.button() == Qt::RightButton) {

    if (item != nullptr) {
      if (!item->isSelected()) {
        // Select the right-clicked item.
        item->setSelected(true);
      }
    }
  }

  return true;
}

/**
 * @brief Constructor.
 * @param view The map view to manage.
 * @param initial_point Point where the drawing started, in view coordinates.
 */
DrawingRectangleState::DrawingRectangleState(MapView& view, const QPoint& initial_point) :
  MapView::State(view),
  initial_point(view.mapToScene(initial_point).toPoint()),
  current_area_item(nullptr),
  initial_selection() {

}

/**
 * @copydoc MapView::State::start
 */
void DrawingRectangleState::start() {

  current_area_item = new QGraphicsRectItem();
  current_area_item->setPen(QPen(Qt::yellow));
  get_scene().addItem(current_area_item);
  initial_selection = get_scene().selectedItems();
}

/**
 * @copydoc MapView::State::stop
 */
void DrawingRectangleState::stop() {

  get_scene().removeItem(current_area_item);
  delete current_area_item;
  current_area_item = nullptr;
}

/**
 * @copydoc MapView::State::mouse_moved
 */
bool DrawingRectangleState::mouse_moved(const QMouseEvent& event) {

  MapView& view = get_view();
  MapScene& scene = get_scene();

  // Compute the selected area.
  QPoint previous_point = current_point;
  current_point = view.mapToScene(event.pos()).toPoint();

  if (current_point == previous_point) {
    // No change.
    return true;
  }

  // The area has changed: recalculate the rectangle.
  QRect area = Rectangle::from_two_points(initial_point, current_point);
  current_area_item->setRect(area);

  // Select items strictly in the rectangle.
  scene.clearSelection();
  QPainterPath path;
  path.addRect(QRect(area.topLeft() - QPoint(1, 1),
                     area.size() + QSize(2, 2)));
  scene.setSelectionArea(path, Qt::ContainsItemBoundingRect);

  // Also restore the initial selection.
  for (QGraphicsItem* item : initial_selection) {
      item->setSelected(true);
  }
  return true;
}

/**
 * @copydoc MapView::State::mouse_released
 */
bool DrawingRectangleState::mouse_released(const QMouseEvent& event) {

  Q_UNUSED(event);

  get_view().start_state_doing_nothing();
  return true;
}

/**
 * @brief Constructor.
 * @param view The map view to manage.
 * @param initial_point Point where the dragging started, in view coordinates.
 */
MovingEntitiesState::MovingEntitiesState(MapView& view, const QPoint& initial_point) :
  MapView::State(view),
  initial_point(Point::round_8(view.mapToScene(initial_point))),
  last_point(this->initial_point),
  total_translation(0, 0) {

}

/**
 * @copydoc MapView::State::mouse_moved
 */
bool MovingEntitiesState::mouse_moved(const QMouseEvent& event) {

  MapView& view = get_view();

  QPoint current_point = Point::round_8(view.mapToScene(event.pos()));
  if (current_point == last_point) {
    // No change after rounding.
    return true;
  }

  // Make the selected entities follow the mouse while dragging.
  QPoint translation = current_point - last_point;
  view.move_selected_entities(translation);

  // Also save the total translation because only the total result will be undoable.
  this->total_translation += translation;
  last_point = current_point;

  return true;
}

/**
 * @copydoc MapView::State::mouse_released
 */
bool MovingEntitiesState::mouse_released(const QMouseEvent& event) {

  Q_UNUSED(event);

  // TODO replace the successive translations by a total one in the undo/redo history.
  get_view().start_state_doing_nothing();

  return true;
}
