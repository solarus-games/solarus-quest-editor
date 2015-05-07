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
#include "gui/edit_entity_dialog.h"
#include "gui/entity_item.h"
#include "gui/enum_menus.h"
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
#include <QApplication>
#include <QClipboard>
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
  ResizingEntitiesState(MapView& view, const EntityIndexes& entities);
  void start() override;
  void mouse_moved(const QMouseEvent& event) override;
  void mouse_released(const QMouseEvent& event) override;

private:
  QRect update_box(const EntityIndex& index, const QPoint& second_xy, bool horizontal_preferred);

  EntityIndexes entities;    /**< Entities to resize. */
  QMap<EntityIndex, QRect>
      old_boxes;                  /**< Bounding rectangle of each entity before resizing. */
  EntityIndex leader_index;       /**< Entity whose resizing follows the cursor position.
                                   * Other ones reproduce an equivalent change. */
  bool first_resize_done;         /**< Whether at least one resizing was done during the state. */
  int num_free_entities;          /**< Number of entities freely resizable (mode ResizeMode::MULTI_DIMENSION_ALL). */

};

/**
 * @brief State of the map view of adding new entities.
 */
class AddingEntitiesState : public MapView::State {

public:
  AddingEntitiesState(MapView& view, EntityModels&& entities, bool guess_layer);
  void start() override;
  void stop() override;
  void mouse_pressed(const QMouseEvent& event) override;
  void mouse_moved(const QMouseEvent& event) override;
  void tileset_selection_changed() override;

private:
  QPoint get_entities_center() const;
  void sort_entities();
  Layer find_best_layer(const EntityModel& entity) const;

  EntityModels entities;                    /**< Entities to be added. */
  std::vector<EntityItem*> entity_items;    /**< Graphic items of entities to be added. */
  QPoint last_point;                        /**< Point where the mouse was last time it moved, in scene coordinates. */
  bool guess_layer;                         /**< Whether the layer should be guessed or kept unchanged. */
};

}  // Anonymous namespace.

/**
 * @brief Creates a map view.
 * @param parent The parent widget or nullptr.
 */
MapView::MapView(QWidget* parent) :
  QGraphicsView(parent),
  map(),
  scene(nullptr),
  view_settings(nullptr),
  zoom(1.0),
  state(),
  common_actions(nullptr),
  edit_action(nullptr),
  resize_action(nullptr),
  convert_tiles_action(nullptr),
  set_layer_actions(),
  bring_to_front_action(nullptr),
  bring_to_back_action(nullptr),
  remove_action(nullptr) {

  setAlignment(Qt::AlignTop | Qt::AlignLeft);

  ViewSettings* view_settings = new ViewSettings(this);
  set_view_settings(*view_settings);

  // Necessary because we draw a custom background.
  setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

  // Initialize actions.
  build_context_menu_actions();
}

/**
 * @brief Returns the map represented in this view.
 * @return The map model or nullptr if none was set.
 */
MapModel* MapView::get_map() {
  return map.data();
}

/**
 * @brief Sets the map to represent in this view.
 * @param map The map model, or nullptr to remove any model.
 * This class does not take ownership on the map.
 * The map can be deleted safely.
 */
void MapView::set_map(MapModel* map) {

  if (this->map != nullptr) {
    this->map = nullptr;
    this->scene = nullptr;
  }

  this->map = map;

  if (map != nullptr) {
    // Create the scene from the map.
    scene = new MapScene(*map, this);
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
 * @return The scene or nullptr if no map was set.
 */
MapScene* MapView::get_scene() {
  return scene;
}

/**
 * @brief Returns the view settings for this map view.
 * @return The view settings, or nullptr if none were set.
 */
const ViewSettings* MapView::get_view_settings() const {
  return view_settings;
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
  connect(this->view_settings, SIGNAL(grid_size_changed(QSize)),
          this, SLOT(update_grid_visibility()));
  connect(this->view_settings, SIGNAL(grid_style_changed(GridStyle)),
          this, SLOT(update_grid_visibility()));
  connect(this->view_settings, SIGNAL(grid_color_changed(QColor)),
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
 * @brief Returns the common actions of the editor.
 * @return The common actions or nullptr if unset.
 */
const QMap<QString, QAction*>* MapView::get_common_actions() const {
  return common_actions;
}

/**
 * @brief Sets the common actions of the editor.
 *
 * This function should be called at initialization time to make actions
 * available in the context menu.
 *
 * @param common_actions The common actions.
 */
void MapView::set_common_actions(const QMap<QString, QAction*>* common_actions) {
  this->common_actions = common_actions;
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

  const EntityIndexes selection = get_selected_entities();
  if (!are_entities_resizable(selection)) {
    // The selection is empty or not resizable.
    start_state_doing_nothing();
  }

  set_state(std::unique_ptr<State>(new ResizingEntitiesState(*this, selection)));
}

/**
 * @brief Moves to the state of adding new entities.
 * @param entities The entities to be added.
 * They must not belong to the map yet.
 * @param guess_layer Whether a layer should be guessed from the preferred
 * layer of entities and the mouse position.
 */
void MapView::start_state_adding_entities(EntityModels&& entities, bool guess_layer) {

  set_state(std::unique_ptr<State>(new AddingEntitiesState(
       *this,
       std::move(entities),
       guess_layer)));
}

/**
 * @brief Moves to the state of adding new entities, with new tiles
 * corresponding to the selected patterns of the tileset.
 */
void MapView::start_adding_entities_from_tileset_selection() {

  MapModel* map = get_map();
  if (map == nullptr) {
    return;
  }

  TilesetModel* tileset = map->get_tileset_model();
  if (tileset == nullptr) {
    return;
  }

  // Create a tile from each selected pattern.
  // Arrange the relative position of tiles as in the tileset.
  EntityModels tiles;
  const QList<int>& pattern_indexes = tileset->get_selected_indexes();
  if (pattern_indexes.isEmpty()) {
    return;
  }

  bool has_common_preferred_layer = true;
  Layer common_preferred_layer = tileset->get_pattern_default_layer(pattern_indexes.first());
  for (int pattern_index : pattern_indexes) {
    QString pattern_id = tileset->index_to_id(pattern_index);
    if (pattern_id.isEmpty()) {
      continue;
    }

    // Create a tile from the pattern.
    QRect pattern_frame = tileset->get_pattern_frame(pattern_index);
    EntityModelPtr tile = EntityModel::create(*map, EntityType::TILE);
    tile->set_field("pattern", pattern_id);
    tile->set_size(pattern_frame.size());
    tile->set_xy(pattern_frame.topLeft());
    Layer preferred_layer = tileset->get_pattern_default_layer(pattern_index);
    tile->set_layer(preferred_layer);
    tiles.emplace_back(std::move(tile));

    // Also check if they all have the same preferred layer.
    if (preferred_layer != common_preferred_layer) {
      has_common_preferred_layer = false;
    }
  }

  // Don't try to choose other layers if they are different at start.
  bool guess_layer = has_common_preferred_layer;

  start_state_adding_entities(std::move(tiles), guess_layer);
}

/**
 * @brief Returns whether at least one entity of a list is resizable.
 * @param indexes Indexes of entities to resize.
 * @return @c true at least one is resizable.
 */
bool MapView::are_entities_resizable(const EntityIndexes& indexes) const {

  for (const EntityIndex& index : indexes) {
    if (map->get_entity(index).is_resizable()) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Creates all actions to be used by context menus.
 */
void MapView::build_context_menu_actions() {

  edit_action = new QAction(
        tr("Edit"), this);
  edit_action->setShortcut(Qt::Key_Return);
  edit_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(edit_action, SIGNAL(triggered()),
          this, SLOT(edit_selected_entity()));
  addAction(edit_action);

  resize_action = new QAction(
        tr("Resize"), this);
  resize_action->setShortcut(tr("R"));
  resize_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(resize_action, &QAction::triggered, [&]() {
    start_state_resizing_entities();
  });
  addAction(resize_action);

  convert_tiles_action = new QAction(
        tr("Convert to dynamic tile"), this);
  connect(convert_tiles_action, SIGNAL(triggered()),
          this, SLOT(convert_selected_tiles()));
  addAction(convert_tiles_action);

  set_layer_actions = EnumMenus<Layer>::create_actions(
      *this,
      EnumMenuCheckableOption::CHECKABLE_EXCLUSIVE,
      [&](Layer layer) {
        emit set_entities_layer_requested(get_selected_entities(), layer);
      }
  );

  bring_to_front_action = new QAction(
        tr("Bring to front"), this);
  bring_to_front_action->setShortcut(tr("T"));
  bring_to_front_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(bring_to_front_action, &QAction::triggered, [&]() {
    emit bring_entities_to_front_requested(get_selected_entities());
  });
  addAction(bring_to_front_action);

  bring_to_back_action = new QAction(
        tr("Bring to back"), this);
  bring_to_back_action->setShortcut(tr("B"));
  bring_to_back_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(bring_to_back_action, &QAction::triggered, [&]() {
    emit bring_entities_to_back_requested(get_selected_entities());
  });
  addAction(bring_to_back_action);

  remove_action = new QAction(
        QIcon(":/images/icon_delete.png"), tr("Delete"), this);
  remove_action->setShortcut(QKeySequence::Delete);
  remove_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(remove_action, SIGNAL(triggered()),
          this, SLOT(remove_selected_entities()));
  addAction(remove_action);
}

/**
 * @brief Creates a context menu for the selected entities.
 * @return A context menu.
 */
QMenu* MapView::create_context_menu() {

  // Layout of the context menu (line breaks are separators):
  //
  // Edit, Resize, Direction
  // Convert to dynamic/static tile(s)
  // Cut, Copy, Paste
  // Low layer, Intermediate Layer, High Layer, Bring to front, Bring to back
  // Delete

  QMenu* menu = new QMenu(this);
  const EntityIndexes& indexes = get_selected_entities();

  // Edit.
  const bool single_selection = indexes.size() <= 1;
  Q_ASSERT(edit_action != nullptr);
  edit_action->setEnabled(single_selection);
  menu->addAction(edit_action);

  // Resize.
  const bool resizable = are_entities_resizable(indexes);
  resize_action->setEnabled(resizable);
  menu->addAction(resize_action);

  // Direction.
  QMenu* direction_menu = create_direction_context_menu(indexes);
  Q_ASSERT(direction_menu != nullptr);
  menu->addMenu(direction_menu);
  menu->addSeparator();

  // Convert to dynamic/static tile(s).
  EntityType type;
  const bool show_convert_tiles_action = map->is_common_type(indexes, type) &&
      (type == EntityType::TILE || type == EntityType::DYNAMIC_TILE);
  if (show_convert_tiles_action) {
    QString text;
    if (type == EntityType::TILE) {
      text = single_selection ? tr("Convert to dynamic tile") : tr("Convert to dynamic tiles");
    }
    else {
      text = single_selection ? tr("Convert to static tile") : tr("Convert to static tiles");
    }
    convert_tiles_action->setText(text);
    menu->addAction(convert_tiles_action);
    menu->addSeparator();
  }

  // Cut, copy, paste.
  if (common_actions != nullptr) {
    // Global actions are available.
    menu->addAction(common_actions->value("cut"));
    menu->addAction(common_actions->value("copy"));
    menu->addAction(common_actions->value("paste"));
    menu->addSeparator();
  }

  // Layer.
  Layer common_layer = Layer::LAYER_LOW;
  bool has_common_layer = map->is_common_layer(indexes, common_layer);
  for (int i = 0; i < Layer::LAYER_NB; ++i) {
    Layer current_layer = static_cast<Layer>(i);
    QAction* action = set_layer_actions[i];
    action->setChecked(false);
    action->setText(LayerTraits::get_friendly_name(current_layer));
    if (has_common_layer && current_layer == common_layer) {
      action->setChecked(true);
      // Add a checkmark (there is none when there is already an icon).
      action->setText("\u2714 " + action->text());
    }
    menu->addAction(action);
  }

  // Bring to front/back.
  menu->addAction(bring_to_front_action);
  menu->addAction(bring_to_back_action);
  menu->addSeparator();

  // Remove.
  menu->addAction(remove_action);

  return menu;
}

/**
 * @brief Creates a context menu to select the direction of entities.
 *
 * Returns a disabled menu if the direction rules of the given entities are incompatible.
 *
 * @param indexes Indexes of entity to treat.
 * @return The direction context menu.
 */
QMenu* MapView::create_direction_context_menu(const EntityIndexes& indexes) {

  QMenu* menu = new QMenu(tr("Direction"), this);

  int num_directions = 0;
  QString no_direction_text;
  if (!map->is_common_direction_rules(indexes, num_directions, no_direction_text)) {
    // Directions rules are incompatible.
    menu->setEnabled(false);
    return menu;
  }

  if (num_directions == 0) {
    // There is no direction field on these entities.
    menu->setEnabled(false);
    return menu;
  }

  QStringList texts;
  if (num_directions == 4) {
    texts = QStringList{
      tr("Right"),
      tr("Up"),
      tr("Left"),
      tr("Down")
    };
  }
  else if (num_directions == 8) {
    texts = QStringList{
      tr("Right"),
      tr("Right-up"),
      tr("Up"),
      tr("Left-up"),
      tr("Left"),
      tr("Left-down"),
      tr("Down"),
      tr("Right-down"),
    };
  }
  else {
    for (int i = 0; i < num_directions; ++num_directions) {
      texts.append(QString::number(i));
    }
  }

  // Create the actions.
  if (!no_direction_text.isEmpty()) {
    // Special no-direction value.
    QAction* action = new QAction(no_direction_text, menu);
    action->setCheckable(true);
    connect(action, &QAction::triggered, [=]() {
      emit set_entities_direction_requested(indexes, -1);
    });
    menu->addAction(action);
  }
  for (int i = 0; i < num_directions; ++i) {
    // Normal direction.
    QAction* action = new QAction(texts[i], menu);
    action->setCheckable(true);
    connect(action, &QAction::triggered, [=]() {
      emit set_entities_direction_requested(indexes, i);
    });
    menu->addAction(action);
  }

  // Check the common direction if any.
  int direction = -1;
  if (map->is_common_direction(indexes, direction)) {
    int i = direction;
    if (!no_direction_text.isEmpty()) {
      ++i;
    }
    menu->actions().at(i)->setChecked(true);
  }

  return menu;
}


/**
 * @brief Copies the selected entities to the clipboard and removes them.
 */
void MapView::cut() {

  if (is_selection_empty()) {
    return;
  }

  copy();
  remove_selected_entities();
}

/**
 * @brief Copies the selected entities to the clipboard.
 */
void MapView::copy() {

  if (map == nullptr) {
    return;
  }

  EntityIndexes indexes = get_selected_entities();
  if (indexes.isEmpty()) {
    return;
  }

  // Sort entities to respect their relative order on the map when pasting.
  std::sort(indexes.begin(), indexes.end());

  QStringList entity_strings;
  for (const EntityIndex& index : indexes) {
    Q_ASSERT(map->entity_exists(index));
    const EntityModel& entity = map->get_entity(index);
    QString entity_string = entity.to_string();
    Q_ASSERT(!entity_string.isEmpty());
    entity_strings << entity_string;
  }

  QString text = entity_strings.join("");
  QApplication::clipboard()->setText(text);
}

/**
 * @brief Adds entities from the clipboard.
 */
void MapView::paste() {

  if (scene == nullptr) {
    return;
  }

  QString text = QApplication::clipboard()->text();
  if (text.isEmpty()) {
    return;
  }

  QStringList entity_strings = text.split(QRegExp("[\n\r]\\}[\n\r]"), QString::SkipEmptyParts);

  EntityModels entities;
  for (int i = 0; i < entity_strings.size(); ++i) {

    QString entity_string = entity_strings.at(i);

    if (entity_string.simplified().isEmpty()) {
      // Only whitespaces: skip.
      continue;
    }

    if (i < entity_strings.size() - 1) {
      entity_string = entity_string + "}";  // Restore the closing brace removed by split().
    }
    EntityModelPtr entity = EntityModel::create(*get_map(), entity_string);
    if (entity == nullptr) {
      // The text data from the clipboard is not a valid entity.
      return;
    }

    entities.push_back(std::move(entity));
  }

  if (entities.empty()) {
    return;
  }

  const bool guess_layer = false;  // Paste entities on the same layer.
  start_state_adding_entities(std::move(entities), guess_layer);
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
 * @brief Slot called when another tileset is set on the map.
 * @param tileset_id The new tileset id.
 */
void MapView::tileset_id_changed(const QString& tileset_id) {

  Q_UNUSED(tileset_id);
  if (scene == nullptr) {
    return;
  }
  scene->update();
}

/**
 * @brief Draws the map view.
 * @param event The paint event.
 */
void MapView::paintEvent(QPaintEvent* event) {

  QGraphicsView::paintEvent(event);

  if (view_settings == nullptr || !view_settings->is_grid_visible()) {
    return;
  }

  // Get the rect and convert to the scene.
  QRect rect = mapToScene(event->rect()).boundingRect().toRect();

  // Get grid and margin.
  QSize grid = view_settings->get_grid_size();
  QSize margin = scene != nullptr ? scene->get_margin_size() : QSize(0, 0);

  // Adjust the rect.
  rect.setTopLeft(QPoint(
    (margin.width() % grid.width()) - grid.width(),
    (margin.height() % grid.height()) - grid.height()));

  // Convert the rect from the scene.
  rect = mapFromScene(rect).boundingRect();
  grid *= zoom;

  // Draw the grid.
  QPainter painter(viewport());
  GuiTools::draw_grid(
    painter, rect, grid, view_settings->get_grid_color(),
    view_settings->get_grid_style());
}

/**
 * @brief Receives a mouse press event.
 * @param event The event to handle.
 */
void MapView::mousePressEvent(QMouseEvent* event) {

  if (map == nullptr || get_scene() == nullptr) {
    return;
  }

  state->mouse_pressed(*event);

  // Don't forward the event to the parent because it would select the item
  // clicked. We only do this explicitly from specific states.
}

/**
 * @brief Receives a mouse release event.
 * @param event The event to handle.
 */
void MapView::mouseReleaseEvent(QMouseEvent* event) {

  if (map != nullptr && get_scene() != nullptr) {
    state->mouse_released(*event);
  }

  QGraphicsView::mouseReleaseEvent(event);
}

/**
 * @brief Receives a mouse move event.
 * @param event The event to handle.
 */
void MapView::mouseMoveEvent(QMouseEvent* event) {

  if (map != nullptr && get_scene() != nullptr) {
    state->mouse_moved(*event);
  }

  // The parent class tracks the mouse movements for internal needs
  // such as anchoring the viewport to the mouse when zooming.
  QGraphicsView::mouseMoveEvent(event);
}

/**
 * @brief Receives a mouse double click event.
 * @param event The event to handle.
 */
void MapView::mouseDoubleClickEvent(QMouseEvent* event) {

  if (get_num_selected_entities() == 1) {

    QList<QGraphicsItem*> items_under_mouse = items(
          QRect(event->pos(), QSize(1, 1)),
          Qt::IntersectsItemBoundingRect  // Pick transparent items too.
    );
    if (!items_under_mouse.isEmpty()) {
      QGraphicsItem* item = items_under_mouse.first();
      EntityModel* entity = scene->get_entity_from_item(*item);
      if (entity != nullptr) {
        edit_selected_entity();
      }
    }
  }
}

/**
 * @brief Receives a context menu event.
 * @param event The event to handle.
 */
void MapView::contextMenuEvent(QContextMenuEvent* event) {

  if (map == nullptr || get_scene() == nullptr) {
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
 * @brief Returns whether the selection is empty.
 * @return @c true if the number of selected entities is zero.
 */
bool MapView::is_selection_empty() const {

  if (scene == nullptr) {
    return true;
  }

  return scene->selectedItems().isEmpty();
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
EntityIndexes MapView::get_selected_entities() const {

  if (scene == nullptr) {
    return EntityIndexes();
  }

  return scene->get_selected_entities();
}

/**
 * @brief Selects the specified entities and unselect the rest.
 * @param indexes Indexes of the entities to make selecteded.
 */
void MapView::set_selected_entities(const EntityIndexes& indexes) {

  if (scene == nullptr) {
    return;
  }

  scene->set_selected_entities(indexes);
}

/**
 * @brief Selects the specified entity and unselect the rest.
 * @param index Index of the entity to make selecteded.
 */
void MapView::set_selected_entity(const EntityIndex& index) {

  EntityIndexes indexes;
  indexes << index;
  set_selected_entities(indexes);
}

/**
 * @brief Creates copies of all selected entities.
 *
 * The created copies are not on the map.
 *
 * @return The created copies.
 */
EntityModels MapView::clone_selected_entities() const {

  EntityModels clones;
  EntityIndexes indexes = get_selected_entities();
  for (const EntityIndex& index : indexes) {
    EntityModelPtr clone = EntityModel::clone(*map, index);
    clones.emplace_back(std::move(clone));
  }
  return clones;
}

/**
 * @brief Returns the index of the entity under the cursor if any.
 * @return The index of the entity under the cursor of an invalid index.
 */
EntityIndex MapView::get_entity_index_under_cursor() const {

  if (scene == nullptr) {
    return EntityIndex();
  }

  QPoint xy = mapFromGlobal(QCursor::pos());
  if (xy.x() < 0 ||
      xy.x() >= width() ||
      xy.y() < 0 ||
      xy.y() >= height()) {
    // The mouse is outside the widget.
    return EntityIndex();
  }

  QGraphicsItem* item = itemAt(xy);
  if (item == nullptr) {
    // No entity under the mouse.
    return EntityIndex();
  }

  EntityModel* entity = scene->get_entity_from_item(*item);
  if (entity == nullptr) {
    // Not an entity.
    return EntityIndex();
  }

  return entity->get_index();
}

/**
 * @brief Opens a dialog to edit the selected entity.
 *
 * Does nothing if the number of selected entities is not 1.
 */
void MapView::edit_selected_entity() {

  if (map == nullptr) {
    return;
  }

  EntityIndexes indexes = get_selected_entities();
  if (indexes.size() != 1) {
    return;
  }

  EntityIndex index = indexes.first();
  EditEntityDialog dialog(map->get_entity(index));
  int result = dialog.exec();
  if (result == QDialog::Rejected) {
    return;
  }

  EntityModelPtr entity_after = std::move(dialog.get_entity_after());
  emit edit_entity_requested(index, entity_after);
}

/**
 * @brief Converts the selected tiles to dynamic tiles or to normal tiles.
 */
void MapView::convert_selected_tiles() {

  emit convert_tiles_requested(get_selected_entities());
}

/**
 * @brief Requests to resize the given entities with the specified bounding boxes.
 * @param translation XY coordinates to add.
 * @param allow_merge_to_previous @c true to merge this move with the previous one if any.
 */
void MapView::move_selected_entities(const QPoint& translation, bool allow_merge_to_previous) {

  emit move_entities_requested(get_selected_entities(), translation, allow_merge_to_previous);
}

/**
 * @brief Requests to move the selected entities with the specified translation.
 * @param boxes The new bounding box to set to each entity.
 * @param allow_merge_to_previous @c true to merge this resizing with the previous one if any.
 */
void MapView::resize_entities(const QMap<EntityIndex, QRect>& boxes, bool allow_merge_to_previous) {

  emit resize_entities_requested(boxes, allow_merge_to_previous);
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
 * @brief Returns the map view managed by this state.
 * @return The map view.
 */
const MapView& MapView::State::get_view() const {
  return view;
}

/**
 * @overload
 *
 * Non-const version.
 */
MapView& MapView::State::get_view() {
  return view;
}

/**
 * @brief Returns the map scene managed by this state.
 * @return The map scene.
 */
const MapScene& MapView::State::get_scene() const {

  return *view.get_scene();
}

/**
 * @overload
 *
 * Non-const version.
 */
MapScene& MapView::State::get_scene() {

  return *view.get_scene();
}

/**
 * @brief Returns the map model represented in the view.
 * @return The map model.
 */
const MapModel& MapView::State::get_map() const {
  return *view.get_map();
}

/**
 * @overload
 *
 * Non-const version.
 */
MapModel& MapView::State::get_map() {
  return *view.get_map();
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
  if (view.is_selection_empty()) {
    return;
  }

  QMenu* menu = view.create_context_menu();
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
  current_area_item->setZValue(4);
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
 * @param entities The entities to resize.
 */
ResizingEntitiesState::ResizingEntitiesState(
    MapView& view, const EntityIndexes& entities) :
  MapView::State(view),
  entities(entities),
  old_boxes(),
  leader_index(),
  first_resize_done(false),
  num_free_entities(0) {
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

    // Count entities freely resizable.
    if (entity.get_resize_mode() == ResizeMode::MULTI_DIMENSION_ALL) {
      ++num_free_entities;
    }
  }

  Q_ASSERT(leader_index.is_valid());
}

/**
 * @copydoc MapView::State::mouse_moved
 */
void ResizingEntitiesState::mouse_moved(const QMouseEvent& event) {

  MapView& view = get_view();
  QPoint current_point = view.mapToScene(event.pos()).toPoint() - MapScene::get_margin_top_left();

  QMap<EntityIndex, QRect> new_boxes;
  const QRect& old_leader_box = old_boxes.value(leader_index);

  // Choose once for all entitites the preferred dimension to use
  // in case resizing is constrained.
  QPoint leader_distance_to_mouse = current_point - old_leader_box.bottomRight();
  leader_distance_to_mouse = QPoint(
        qAbs(leader_distance_to_mouse.x()),
        qAbs(leader_distance_to_mouse.y())
  );
  bool horizontal_preferred = leader_distance_to_mouse.x() > leader_distance_to_mouse.y();

  // Compute the new size and position of each entity.
  bool changed = false;
  for (auto it = old_boxes.constBegin(); it != old_boxes.end(); ++it) {
    const EntityIndex& index = it.key();
    const QRect& old_box = it.value();

    QPoint leader_offset(old_box.bottomRight() - old_leader_box.bottomRight());
    QRect new_box = update_box(index, leader_offset + current_point, horizontal_preferred);
    new_boxes.insert(index, new_box);
    changed |= new_box != old_box;
  }

  const bool allow_merge_to_previous = first_resize_done;
  view.resize_entities(new_boxes, allow_merge_to_previous);
  first_resize_done = true;
}

/**
 * @copydoc MapView::State::mouse_released
 */
void ResizingEntitiesState::mouse_released(const QMouseEvent& event) {

  MapView& view = get_view();
  if (event.button() == Qt::RightButton) {
    // The right button was pressed during the resize:
    // add copies of the entities.
    EntityModels clones = view.clone_selected_entities();

    if (clones.size() == 1 && first_resize_done) {
      // A real resize was done.
      // We are probably adding successive entities like tiles and resizing
      // each one (contrary to just copying entities).
      // In this case, each new clone should be reset to the base size.
      const EntityModelPtr& clone = *clones.begin();
      clone->set_size(clone->get_base_size());
    }
    const bool guess_layer = false;
    view.start_state_adding_entities(std::move(clones), guess_layer);
  }
  else {
    get_view().start_state_doing_nothing();
  }
}

/**
 * @brief Updates with new coordinates the rectangle of one entity.
 * @param index Index of the entity to resize.
 * @param second_xy Coordinate of the second point of the rectangle to set for this entity,
 * in map coordinates.
 * @param horizontal_preferred When only one of both dimensions can be resized, whether
 * this should be the horizontal or vertical dimension.
 * @return A new bounding box for the entity.
 * Returns a null rectangle in case of error.
 */
QRect ResizingEntitiesState::update_box(const EntityIndex& index, const QPoint& second_xy, bool horizontal_preferred) {

  MapModel& map = get_map();
  Q_ASSERT(map.entity_exists(index));
  EntityModel& entity = map.get_entity(index);

  const QSize& base_size = entity.get_base_size();
  int base_width = base_size.width();
  int base_height = base_size.height();

  ResizeMode resize_mode = entity.get_resize_mode();
  if (num_free_entities > 1 && resize_mode == ResizeMode::MULTI_DIMENSION_ALL) {
    // Multiple resize: restrict the resizing to only one dimension.
    resize_mode = ResizeMode::MULTI_DIMENSION_ONE;
  }

  const QRect& old_box = old_boxes.value(index);
  if (resize_mode == ResizeMode::NONE) {
    return old_box;
  }

  QPoint point_a = old_box.topLeft();
  QPoint point_b = second_xy;
  // A is the original point of the rectangle we are drawing.
  // B is the second point of the rectangle, determined by the mouse position.

  // We want to extend the entity's rectangle with units of base_size() from A to B.
  QPoint diff = point_b - point_a;
  int sign_x = diff.x() >= 0 ? 1 : -1;
  int sign_y = diff.y() >= 0 ? 1 : -1;

  // Calculate the coordinates of B such that the size of the
  // rectangle from A to B is a multiple of base_size.
  point_b += QPoint(
        sign_x * (base_width - ((qAbs(diff.x()) + base_width) % base_width)),
        sign_y * (base_height - ((qAbs(diff.y()) + base_height) % base_height))
  );

  QPoint abs_diff = QPoint(
        qAbs(point_b.x() - point_a.x()),
        qAbs(point_b.y() - point_a.y())
  );
  if (resize_mode == ResizeMode::SQUARE) {
    int length = qMax(abs_diff.x(), abs_diff.y());  // Length of the square.
    point_b = QPoint(
          point_a.x() + sign_x * length,
          point_a.y() + sign_y * length
    );
  }
  else {
    // Make sure that the entity is extended only in allowed directions,
    // and that the size is never zero.
    if (resize_mode == ResizeMode::SINGLE_DIMENSION) {
      resize_mode = horizontal_preferred ? ResizeMode::HORIZONTAL_ONLY : ResizeMode::VERTICAL_ONLY;
    }

    if (resize_mode == ResizeMode::VERTICAL_ONLY) {
      // Extensible only vertically with the x coordinate of B fixed to the base width.
      point_b.setX(point_a.x() + base_width);
    }
    else if (resize_mode == ResizeMode::MULTI_DIMENSION_ONE &&
             !horizontal_preferred) {
      // Extensible only vertically with the x coordinate of B fixed to the current width.
      point_b.setX(point_a.x() + old_box.width());
    }
    else {
      // Extensible horizontally.
      if (point_b.x() <= point_a.x()) {
        // B is actually before A: in this case, set A to its right coordinate.
        point_a.setX(point_a.x() + base_width);
      }
    }

    if (resize_mode == ResizeMode::HORIZONTAL_ONLY) {
      // Extensible only horizontally with the y coordinate of B fixed to the base height.
      point_b.setY(point_a.y() + base_height);
    }
    else if (resize_mode == ResizeMode::MULTI_DIMENSION_ONE &&
             horizontal_preferred) {
      // Extensible only horizontally with the y coordinate of B fixed to the current height.
      point_b.setY(point_a.y() + old_box.height());
    }
    else {
      // Extensible vertically.
      if (point_b.y() <= point_a.y()) {
        // B is actually before A: in this case, set A to its bottom coordinate.
        point_a.setY(point_a.y() + base_height);
      }
    }
  }

  // Compute the final bounding box from A to B.
  // Note that A is not necessarily the top-left corner of the rectangle.
  QPoint top_left(
        qMin(point_a.x(), point_b.x()),
        qMin(point_a.y(), point_b.y())
  );
  QSize size(
        qAbs(point_b.x() - point_a.x()),
        qAbs(point_b.y() - point_a.y())
  );

  return QRect(top_left, size);
}

/**
 * @brief Constructor.
 * @param view The map view to manage.
 * @param entities The entities to be added to the map.
 * @param guess_layer Whether a layer should be guessed from the preferred
 * layer of entities and the mouse position.
 * If @c false, they will be added on their layer indicated by
 * EntityModel::get_layer().
 */
AddingEntitiesState::AddingEntitiesState(MapView& view, EntityModels&& entities, bool guess_layer) :
  MapView::State(view),
  entities(std::move(entities)),
  entity_items(),
  guess_layer(guess_layer) {

  for (const EntityModelPtr& entity : this->entities) {
    EntityItem* item = new EntityItem(*entity);
    item->setZValue(Layer::LAYER_NB);
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
 * @brief Ensures that the entities to be added are correctly sorted.
 *
 * The ones on lower layers come first, and on the same layer,
 * the non-dynamic ones come first.
 */
void AddingEntitiesState::sort_entities() {

  std::map<Layer, EntityModels> entities_by_layer;
  for (EntityModelPtr& entity : entities) {
    if (!entity->is_dynamic()) {  // Non-dynamic ones first.
      Layer layer = entity->get_layer();
      entities_by_layer[layer].emplace_back(std::move(entity));
    }
  }
  for (EntityModelPtr& entity : entities) {
    if (entity != nullptr && entity->is_dynamic()) {  // Non-dynamic ones first.
      Layer layer = entity->get_layer();
      entities_by_layer[layer].emplace_back(std::move(entity));
    }
  }
  entities.clear();
  for (int i = 0; i < Layer::LAYER_NB; ++i) {
    Layer layer = static_cast<Layer>(i);
    for (EntityModelPtr& entity : entities_by_layer[layer]) {
      entities.emplace_back(std::move(entity));
    }
  }
}

/**
 * @copydoc MapView::State::mouse_pressed
 */
void AddingEntitiesState::mouse_pressed(const QMouseEvent& event) {

  Q_UNUSED(event);
  MapModel& map = get_map();
  MapView& view = get_view();

  // Store the number of tiles and dynamic entities of each layer,
  // because every entity added will increment one of them.
  QMap<Layer, int> num_tiles_by_layer;     // Index where to append a tile.
  QMap<Layer, int> num_dynamic_entities_by_layer;  // Index where to append a dynamic entity.
  for (int i = 0; i < Layer::LAYER_NB; ++i) {
    Layer layer = static_cast<Layer>(i);
    num_tiles_by_layer[layer] = map.get_num_tiles(layer);
    num_dynamic_entities_by_layer[layer] = map.get_num_dynamic_entities(layer);
  }

  // Determine the best layer of each entity.
  for (EntityModelPtr& entity : entities) {
    Layer layer = find_best_layer(*entity);
    entity->set_layer(layer);
  }
  // Now that their layer is known, sort them
  // to compute correct indexes below.
  sort_entities();

  // Make entities ready to be added at their specific index.
  AddableEntities addable_entities;
  EntityIndex previous_index;
  for (EntityModelPtr& entity : entities) {
    Q_ASSERT(entity != nullptr);
    Layer layer = entity->get_layer();

    int i = 0;
    if (entity->is_dynamic()) {
      i = num_tiles_by_layer[layer] + num_dynamic_entities_by_layer[layer];
      ++num_dynamic_entities_by_layer[layer];
    }
    else {
      i = num_tiles_by_layer[layer];
      ++num_tiles_by_layer[layer];
    }

    EntityIndex index = { layer, i };
    if (previous_index.is_valid()) {
      // Double-check that we are traversing entities in ascending order.
      // (If not, then sort_entities() above did not make its job and we risk
      // invalid indexes).
      Q_ASSERT(index > previous_index);
    }
    previous_index = index;
    addable_entities.emplace_back(std::move(entity), index);
  }

  // Add them.
  view.add_entities_requested(addable_entities);

  // Start resizing the newly added entities
  // (until the mouse button is released).
  view.start_state_resizing_entities();
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

/**
 * @brief Determines the appropriate layer where to add an entity.
 * @param entity The entity to add.
 * @return The layer.
 */
Layer AddingEntitiesState::find_best_layer(const EntityModel& entity) const {

  if (!guess_layer) {
    return entity.get_layer();
  }

  Layer layer_under = get_scene().get_layer_in_rectangle(
        entity.get_bounding_box()
  );
  if (!entity.get_has_preferred_layer()) {
    // The entity has no preferred layer.
    return layer_under;
  }

  // The entity has a preferred layer:
  // see if there is something above its preferred layer.
  Layer preferred_layer = entity.get_preferred_layer();
  if (static_cast<int>(layer_under) > static_cast<int>(preferred_layer)) {
      // Don't use the preferred layer in this case.
      return layer_under;
  }

  return preferred_layer;
}
