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
#include "entities/tile.h"
#include "gui/entity_item.h"
#include "gui/gui_tools.h"
#include "gui/map_scene.h"
#include "gui/map_view.h"
#include "gui/mouse_coordinates_tracking_tool.h"
#include "gui/pan_tool.h"
#include "gui/zoom_tool.h"
#include "point.h"
#include "rectangle.h"
#include "tileset_model.h"
#include "view_settings.h"
#include <QAction>
#include <QDebug>
#include <QGraphicsItem>
#include <QMap>
#include <QMenu>
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

  void mouse_pressed(const QMouseEvent& event) override;
  void context_menu_requested(const QPoint& where) override;
  void tileset_selection_changed() override;

};

/**
 * @brief State of the map view of drawing a selection rectangle.
 */
class DrawingRectangleState : public MapView::State {

public:
  DrawingRectangleState(MapView& view, const QPoint& initial_point);

  void start() override;
  void stop() override;

  void mouse_moved(const QMouseEvent& event) override;
  void mouse_released(const QMouseEvent& event) override;

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

  void mouse_moved(const QMouseEvent& event) override;
  void mouse_released(const QMouseEvent& event) override;

private:
  QPoint initial_point;      /**< Point where the dragging started, in scene coordinates. */
  QPoint last_point;         /**< Point where the mouse was last time it moved, in scene coordinates. */
  bool first_move_done;      /**< Whether at least one move was done during the state. */
};

/**
 * @brief State of the map view of resizing entities.
 */
class ResizingEntitiesState : public MapView::State {

public:
  ResizingEntitiesState(MapView& view, const QList<EntityIndex>& entities, ResizeMode resize_mode);  
  void start() override;
  void mouse_moved(const QMouseEvent& event) override;

private:
  void update_box(const EntityIndex& index, const QPoint& second_xy);

  QList<EntityIndex> entities;    /**< Entities to resize. */
  QMap<EntityIndex, QRect>
      old_boxes;                  /**< Bounding rectangle of each entity before resizing. */
  EntityIndex leader_index;       /**< Entity whose resizing follows the cursor position.
                                   * Other ones reproduce an equivalent change. */
  //ResizeMode resize_mode;         /**< How the resizing can be done. */
  QPoint last_point;              /**< Point where the mouse was last time it moved, in scene coordinates. */
  //bool first_move_done;           /**< Whether at least one move was done during the state. */

};

/**
 * @brief State of the map view of adding new entities.
 */
class AddingEntitiesState : public MapView::State {

public:
  AddingEntitiesState(MapView& view, EntityModels&& entities);
  void start() override;
  void stop() override;
  void mouse_pressed(const QMouseEvent& event) override;
  void mouse_moved(const QMouseEvent& event) override;
  void tileset_selection_changed() override;

private:
  QPoint get_entities_center() const;

  EntityModels entities;                    /**< Entities to be added. */
  std::vector<EntityItem*> entity_items;    /**< Graphic items of entities to be added. */
  QPoint last_point;                        /**< Point where the mouse was last time it moved, in scene coordinates. */
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
  zoom(1.0),
  state(),
  resize_entities_action(nullptr),
  remove_entities_action(nullptr) {

  setAlignment(Qt::AlignTop | Qt::AlignLeft);

  ViewSettings* view_settings = new ViewSettings(this);
  set_view_settings(*view_settings);

  // Necessary because we draw a custom background.
  setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

  // Initialize actions.
  resize_entities_action = new QAction(
      MapView::tr("Resize"), this);
  resize_entities_action->setShortcut(tr("R"));
  resize_entities_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  QObject::connect(resize_entities_action, &QAction::triggered, [&]() {
    start_state_resizing_entities();
  });
  addAction(resize_entities_action);

  remove_entities_action = new QAction(
        QIcon(":/images/icon_delete.png"), MapView::tr("Delete"), this);
  remove_entities_action->setShortcut(QKeySequence::Delete);
  remove_entities_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  QObject::connect(remove_entities_action, SIGNAL(triggered()),
                   this, SLOT(remove_selected_entities()));
  addAction(remove_entities_action);
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
 * @brief Moves to the state of drawing a rectangle for a selection.
 * @param initial_point Where the user starts drawing the rectangle,
 * in view coordinates.
 */
void MapView::start_state_drawing_rectangle(const QPoint& initial_point) {

  set_state(std::unique_ptr<State>(new DrawingRectangleState(*this, initial_point)));
}

/**
 * @brief Moves to the state of moving the selected entities.
 * @param initial_point Where the user starts dragging the entities,
 */
void MapView::start_state_moving_entities(const QPoint& initial_point) {

  set_state(std::unique_ptr<State>(new MovingEntitiesState(*this, initial_point)));
}

/**
 * @brief Moves to the state of resizing the selected entities.
 *
 * Does nothing if there is no selected entity or if the selection is not
 * resizable.
 */
void MapView::start_state_resizing_entities() {

  const QList<EntityIndex> selection = get_selected_entities();
  ResizeMode resize_mode = get_best_resize_mode(selection);
  if (resize_mode == ResizeMode::NONE) {
    // The selection is empty or not resizable.
    return;
  }

  set_state(std::unique_ptr<State>(new ResizingEntitiesState(*this, selection, resize_mode)));
}

/**
 * @brief Moves to the state of adding new entities.
 * @param entities The entities to be added.
 * They must not belong to the map yet.
 */
void MapView::start_state_adding_entities(EntityModels&& entities) {

  set_state(std::unique_ptr<State>(new AddingEntitiesState(*this, std::move(entities))));
}

/**
 * @brief Moves to the state of adding new entities, with new tiles
 * corresponding to the selected patterns of the tileset.
 */
void MapView::start_adding_entities_from_tileset_selection() {

  MapModel* map = get_model();
  if (map == nullptr) {
    return;
  }

  TilesetModel* tileset = get_model()->get_tileset_model();
  if (tileset == nullptr) {
    return;
  }

  // Create a tile from each selected pattern.
  // Arrange the relative position of tiles as in the tileset.
  EntityModels tiles;
  const QList<int>& pattern_indexes = tileset->get_selected_indexes();
  for (int pattern_index : pattern_indexes) {
    QString pattern_id = tileset->index_to_id(pattern_index);
    if (pattern_id.isEmpty()) {
      continue;
    }
    QRect pattern_frame = tileset->get_pattern_frame(pattern_index);
    EntityModelPtr tile = EntityModel::create(*map, EntityType::TILE);
    tile->set_field("pattern", pattern_id);
    tile->set_size(pattern_frame.size());
    tile->set_xy(pattern_frame.topLeft());
    tiles.emplace_back(std::move(tile));
  }

  start_state_adding_entities(std::move(tiles));
}

/**
 * @brief Returns the action of resizing selected entities.
 * @return The resize entities action.
 */
QAction& MapView::get_resize_entities_action() {
  return *resize_entities_action;
}

/**
 * @brief Returns the action of removing selected entities.
 * @return The remove entities action.
 */
QAction& MapView::get_remove_entities_action() {
  return *remove_entities_action;
}

/**
 * @brief Returns whether all entities of a list are resizable.
 * @param indexes Indexes of entities to resize.
 * @return @c true if they are all resizable, @c false if at least one is not.
 */
bool MapView::are_entities_resizable(const QList<EntityIndex>& indexes) const {
  return get_best_resize_mode(indexes) != ResizeMode::NONE;
}

/**
 * @brief Determines the most appropriate resize mode for the given entities.
 * @param indexes Indexes of entities to resize.
 * @return The best resize mode.
 */
ResizeMode MapView::get_best_resize_mode(const QList<EntityIndex>& indexes) const {

  if (model == nullptr) {
    return ResizeMode::NONE;
  }

  if (indexes.isEmpty()) {
    return ResizeMode::NONE;
  }

  if (indexes.size() == 1) {
    // Resizing a single entity: simply return its mode.
    const EntityModel& entity = model->get_entity(indexes.first());
    return entity.get_resize_mode();
  }

  // When we resize multiple entities, allow at most to do it only one
  // direction at a time.
  ResizeMode candidate_mode = ResizeMode::SINGLE_DIMENSION;

  // Then see if some entities are more restrictive than that.
  for (const EntityIndex& index : indexes) {

    const EntityModel& entity = model->get_entity(index);
    ResizeMode current_resize_mode = entity.get_resize_mode();
    switch (current_resize_mode) {

    case ResizeMode::NONE:
      // At least one entity is not resizable: no need to check others.
      return ResizeMode::NONE;

    case ResizeMode::HORIZONTAL_ONLY:
      if (candidate_mode == ResizeMode::VERTICAL_ONLY || candidate_mode == ResizeMode::SQUARE) {
        // Incompatible modes.
        return ResizeMode::NONE;
      }
      candidate_mode = ResizeMode::HORIZONTAL_ONLY;
      break;

    case ResizeMode::VERTICAL_ONLY:
      if (candidate_mode == ResizeMode::HORIZONTAL_ONLY || candidate_mode == ResizeMode::SQUARE) {
        // Incompatible modes.
        return ResizeMode::NONE;
      }
      candidate_mode = ResizeMode::VERTICAL_ONLY;
      break;

    case ResizeMode::SQUARE:
      if (candidate_mode != ResizeMode::SQUARE && candidate_mode != ResizeMode::MULTI_DIMENSION) {
        // Incompatible modes.
        return ResizeMode::NONE;
      }

    case ResizeMode::SINGLE_DIMENSION:
      if (candidate_mode == ResizeMode::SQUARE) {
        // Incompatible modes.
        return ResizeMode::NONE;
      }
      break;

    case ResizeMode::MULTI_DIMENSION:
      // No further restriction.
      break;
    }
  }

  return candidate_mode;
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
 * @brief Slot called when the pattern selection of the tileset is changed.
 *
 * Tiles with these new patterns are added if possible.
 */
void MapView::tileset_selection_changed() {

  if (state == nullptr) {
    return;
  }

  state->tileset_selection_changed();
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
    state->mouse_pressed(*event);
  }
}

/**
 * @brief Receives a mouse release event.
 * @param event The event to handle.
 */
void MapView::mouseReleaseEvent(QMouseEvent* event) {

  if (model != nullptr && get_scene() != nullptr) {
    state->mouse_released(*event);
  }
}

/**
 * @brief Receives a mouse move event.
 * @param event The event to handle.
 */
void MapView::mouseMoveEvent(QMouseEvent* event) {

  if (model != nullptr && get_scene() != nullptr) {
    state->mouse_moved(*event);
  }
}

/**
 * @brief Receives a context menu event.
 * @param event The event to handle.
 */
void MapView::contextMenuEvent(QContextMenuEvent* event) {

  if (model == nullptr || get_scene() == nullptr) {
    return;
  }

  QPoint where;
  if (event->pos() != QPoint(0, 0)) {
    where = event->pos() + QPoint(1, 1);
  }
  else {
    QList<QGraphicsItem*> selected_items = scene->selectedItems();
    where = mapFromScene(selected_items.first()->pos() + QPoint(8, 8));
  }

  state->context_menu_requested(viewport()->mapToGlobal(where));
}

/**
 * @brief Returns the number of selected entities.
 * @return The number of selected entities.
 */
int MapView::get_num_selected_entities() const {

  if (scene == nullptr) {
    return 0;
  }

  return scene->selectedItems().size();
}

/**
 * @brief Returns the indexes of selected entities.
 * @return The selected entities.
 */
QList<EntityIndex> MapView::get_selected_entities() const {

  if (scene == nullptr) {
    return QList<EntityIndex>();
  }

  return scene->get_selected_entities();
}

/**
 * @brief Selects the specified entities and unselect the rest.
 * @param indexes Indexes of the entities to make selecteded.
 */
void MapView::set_selected_entities(const QList<EntityIndex>& indexes) {

  if (scene == nullptr) {
    return;
  }

  scene->set_selected_entities(indexes);
}

/**
 * @brief Requests to move the selected entities with the specified translation.
 * @param translation XY coordinates to add.
 * @param allow_merge_to_previous @c true to merge this move with the previous one if any.
 */
void MapView::move_selected_entities(const QPoint& translation, bool allow_merge_to_previous) {

  emit move_entities_requested(get_selected_entities(), translation, allow_merge_to_previous);
}

/**
 * @brief Requests to delete the selected entities.
 */
void MapView::remove_selected_entities() {

  emit remove_entities_requested(get_selected_entities());
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
 */
void MapView::State::mouse_pressed(const QMouseEvent& event) {

  Q_UNUSED(event);
}

/**
 * @brief Called when the mouse is released in the map view during this state.
 *
 * Subclasses can reimplement this function to define what happens.
 *
 * @param event The event to handle.
 */
void MapView::State::mouse_released(const QMouseEvent& event) {

  Q_UNUSED(event);
}

/**
 * @brief Called when the mouse is moved in the map view during this state.
 *
 * Subclasses can reimplement this function to define what happens.
 *
 * @param event The event to handle.
 */
void MapView::State::mouse_moved(const QMouseEvent& event) {

  Q_UNUSED(event);
}

/**
 * @brief Called when a context menu is requested in the map view during this
 * state.
 *
 * Subclasses can reimplement this function to show a context menu.
 *
 * @param where Where to show the context menu, in global coordinates.
 */
void MapView::State::context_menu_requested(const QPoint& where) {

  Q_UNUSED(where);
}

/**
 * @brief Called when the user changes the selection in the tileset.
 *
 * States may start or stop adding entities.
 */
void MapView::State::tileset_selection_changed() {

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
void DoingNothingState::mouse_pressed(const QMouseEvent& event) {

  if (event.button() != Qt::LeftButton && event.button() != Qt::RightButton) {
    return;
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
}

/**
 * @copydoc MapView::State::context_menu_requested
 */
void DoingNothingState::context_menu_requested(const QPoint& where) {

  MapView& view = get_view();
  const QList<EntityIndex>& selection = view.get_selected_entities();
  if (selection.isEmpty()) {
    return;
  }

  QAction& resize_action = view.get_resize_entities_action();
  bool resizable = view.get_best_resize_mode(selection) != ResizeMode::NONE;
  resize_action.setEnabled(resizable);

  QMenu* menu = new QMenu(&view);
  menu->addAction(&resize_action);
  menu->addAction(&view.get_remove_entities_action());

  menu->popup(where);
}

/**
 * @copydoc MapView::State::tileset_selection_changed
 */
void DoingNothingState::tileset_selection_changed() {

  TilesetModel* tileset = get_map().get_tileset_model();
  if (tileset == nullptr) {
    return;
  }
  if (tileset->is_selection_empty()) {
    return;
  }

  // The user just selected some patterns in the tileset: create corresponding tiles.
  get_view().start_adding_entities_from_tileset_selection();
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
void DrawingRectangleState::mouse_moved(const QMouseEvent& event) {

  MapView& view = get_view();
  MapScene& scene = get_scene();

  // Compute the selected area.
  QPoint previous_point = current_point;
  current_point = view.mapToScene(event.pos()).toPoint();

  if (current_point == previous_point) {
    // No change.
    return;
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
  return;
}

/**
 * @copydoc MapView::State::mouse_released
 */
void DrawingRectangleState::mouse_released(const QMouseEvent& event) {

  Q_UNUSED(event);

  get_view().start_state_doing_nothing();
  return;
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
  first_move_done(false) {

}

/**
 * @copydoc MapView::State::mouse_moved
 */
void MovingEntitiesState::mouse_moved(const QMouseEvent& event) {

  MapView& view = get_view();

  QPoint current_point = Point::round_8(view.mapToScene(event.pos()));
  if (current_point == last_point) {
    // No change after rounding.
    return;
  }

  // Make selected entities follow the mouse while dragging.
  QPoint translation = current_point - last_point;
  last_point = current_point;

  // Merge undo actions of successive moves,
  // but don't merge the first one of this state instance to potential previous states.
  const bool allow_merge_to_previous = first_move_done;
  view.move_selected_entities(translation, allow_merge_to_previous);
  first_move_done = true;
}

/**
 * @copydoc MapView::State::mouse_released
 */
void MovingEntitiesState::mouse_released(const QMouseEvent& event) {

  Q_UNUSED(event);

  get_view().start_state_doing_nothing();
}

/**
 * @brief Constructor.
 * @param view The map view to manage.
 */
ResizingEntitiesState::ResizingEntitiesState(
    MapView& view, const QList<EntityIndex>& entities, ResizeMode resize_mode) :
  MapView::State(view),
  entities(entities),
  old_boxes(),
  leader_index(),
  //resize_mode(resize_mode)
  last_point()
//  first_move_done(false)
{
  Q_UNUSED(resize_mode);
}

/**
 * @copydoc MapView::State::start
 */
void ResizingEntitiesState::start() {

  MapView& view = get_view();
  MapModel& map = get_map();
  const QPoint mouse_position_in_view = view.mapFromGlobal(QCursor::pos());
  QPoint mouse_position = view.mapToScene(mouse_position_in_view).toPoint() - MapScene::get_margin_top_left();

  // Choose the leader: it will be the entity whose bottom-right corner
  // is the nearest to the mouse.
  int min_distance = std::numeric_limits<int>::max();
  for (const EntityIndex& index : entities) {
    const EntityModel& entity = map.get_entity(index);
    const QPoint& bottom_right = entity.get_bottom_right();
    int distance = (bottom_right - mouse_position).manhattanLength();
    if (distance < min_distance) {
      leader_index = index;
      min_distance = distance;
    }

    // Also save the initial position of entities.
    old_boxes.insert(index, entity.get_bounding_box());
  }

  Q_ASSERT(leader_index.is_valid());
}

/**
 * @copydoc MapView::State::mouse_moved
 */
void ResizingEntitiesState::mouse_moved(const QMouseEvent& event) {

  MapView& view = get_view();

  QPoint current_point = Point::round_8(view.mapToScene(event.pos()));
  if (current_point == last_point) {
    // No change after rounding.
    return;
  }
  last_point = current_point;

  const QRect& old_master_box = old_boxes.value(leader_index);
  for (auto it = old_boxes.constBegin(); it != old_boxes.end(); ++it) {
    const EntityIndex& index = it.key();
    const QRect& old_box = it.value();

    QPoint master_offset(old_box.bottomRight() - old_master_box.bottomRight());
    update_box(index, master_offset + current_point);
  }
}

/**
 * @brief Updates with new coordinates the rectangle of one entity.
 * @param index Index of the entity to resize.
 * @param second_xy Coordinate of the second point of the rectangle to set for this entity.
 */
void ResizingEntitiesState::update_box(const EntityIndex& index, const QPoint& second_xy) {

  Q_UNUSED(index);
  Q_UNUSED(second_xy);
  // TODO
}

/**
 * @brief Constructor.
 * @param view The map view to manage.
 * @param entities The entities to be added to the map.
 */
AddingEntitiesState::AddingEntitiesState(MapView& view, EntityModels&& entities) :
  MapView::State(view),
  entities(std::move(entities)),
  entity_items() {

  for (const EntityModelPtr& entity : this->entities) {
    EntityItem* item = new EntityItem(*entity);
    entity_items.push_back(item);
  }
}

/**
 * @copydoc MapView::State::start
 */
void AddingEntitiesState::start() {

  MapView& view = get_view();
  QPoint mouse_position = view.mapFromGlobal(QCursor::pos());
  last_point = Point::round_8(view.mapToScene(mouse_position));

  // Determine the center of all entities in their current position.
  QPoint center = get_entities_center();

  // Adds the graphic item of each entity.
  for (EntityItem* item : entity_items) {
    get_scene().addItem(item);
    EntityModel& entity = item->get_entity();
    QPoint top_left_in_group = center - entity.get_top_left();
    QPoint top_left = last_point - top_left_in_group - MapScene::get_margin_top_left();
    top_left = Point::round_8(top_left);
    entity.set_top_left(top_left);
    item->update_xy();
  }
}

/**
 * @copydoc MapView::State::stop
 */
void AddingEntitiesState::stop() {

  for (EntityItem* item : entity_items) {
    get_scene().removeItem(item);
  }
}

/**
 * @brief Computes the center point of all entities to be added.
 * @return The center point.
 */
QPoint AddingEntitiesState::get_entities_center() const {

  QPoint top_left(1e9, 1e9);
  QPoint bottom_right(-1e9, -1e9);
  for (EntityItem* item : entity_items) {
    EntityModel& entity = item->get_entity();
    QRect box = entity.get_bounding_box();
    top_left.setX(qMin(box.left(), top_left.x()));
    top_left.setY(qMin(box.top(), top_left.y()));
    bottom_right.setX(qMax(box.right(), bottom_right.x()));
    bottom_right.setY(qMax(box.bottom(), bottom_right.y()));
  }
  return (top_left + bottom_right) / 2;
}

/**
 * @copydoc MapView::State::mouse_pressed
 */
void AddingEntitiesState::mouse_pressed(const QMouseEvent& event) {

  Q_UNUSED(event);
  MapModel& map = get_map();
  MapView& view = get_view();

  // Add the entities to the map and make them selected.

  AddableEntities addable_entities;

  // TODO Determine the best layer.
  Layer layer = Layer::LAYER_LOW;
  int i = map.get_num_entities(layer);
  for (EntityModelPtr& entity : entities) {
    EntityIndex index = { layer, i };
    ++i;
    addable_entities.emplace_back(std::move(entity), index);
  }
  view.add_entities_requested(addable_entities);

  view.start_state_doing_nothing();
}

/**
 * @copydoc MapView::State::mouse_moved
 */
void AddingEntitiesState::mouse_moved(const QMouseEvent& event) {

  MapView& view = get_view();

  QPoint current_point = Point::round_8(view.mapToScene(event.pos()));
  if (current_point == last_point) {
    // No change after rounding.
    return;
  }

  // Make entities being added follow the mouse.
  QPoint translation = current_point - last_point;
  last_point = current_point;

  for (EntityItem* item : entity_items) {
    EntityModel& entity = item->get_entity();
    QPoint xy = entity.get_xy() + translation;
    entity.set_xy(xy);
    item->update_xy();
  }
}

/**
 * @copydoc MapView::State::tileset_selection_changed
 */
void AddingEntitiesState::tileset_selection_changed() {

  TilesetModel* tileset = get_map().get_tileset_model();
  if (tileset == nullptr) {
    return;
  }
  if (tileset->is_selection_empty()) {
    // Stop adding the tiles that were selected.
    get_view().start_state_doing_nothing();
    return;
  }

  // The user just selected some patterns in the tileset: create corresponding tiles.
  get_view().start_adding_entities_from_tileset_selection();
}
