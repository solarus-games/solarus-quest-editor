/*
 * Copyright (C) 2014-2017 Christopho, Solarus - http://www.solarus-games.org
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
#include "widgets/gui_tools.h"
#include "widgets/pan_tool.h"
#include "widgets/sprite_scene.h"
#include "widgets/sprite_view.h"
#include "widgets/zoom_tool.h"
#include "view_settings.h"
#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>
#include <QtMath>

namespace SolarusEditor {

/**
 * @brief Creates a sprite view.
 * @param parent The parent widget or nullptr.
 */
SpriteView::SpriteView(QWidget* parent) :
  QGraphicsView(parent),
  scene(nullptr),
  delete_direction_action(nullptr),
  state(State::NORMAL),
  view_settings(nullptr),
  zoom(1.0) {

  setAlignment(Qt::AlignTop | Qt::AlignLeft);
  current_area_item.setZValue(2);

  delete_direction_action = new QAction(
        QIcon(":/images/icon_delete.png"), tr("Delete..."), this);
  delete_direction_action->setShortcut(QKeySequence::Delete);
  delete_direction_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(delete_direction_action, SIGNAL(triggered()),
          this, SIGNAL(delete_selected_direction_requested()));
  addAction(delete_direction_action);
  duplicate_direction_action = new QAction(
        QIcon(":/images/icon_copy.png"), tr("Duplicate..."), this);
  // TODO: set a shortcut to duplicate a direction
  connect(duplicate_direction_action, SIGNAL(triggered()),
          this, SLOT(duplicate_selected_direction_requested()));
  addAction(duplicate_direction_action);

  change_num_frames_columns_action = new QAction(
        tr("Change the number of frames/columns"), this);
  // TODO: set a shortcut to changing the number of frames and columns
  connect(change_num_frames_columns_action, SIGNAL(triggered()),
          this, SLOT(change_num_frames_columns_requested()));
  addAction(change_num_frames_columns_action);

  change_num_frames_action = new QAction(
        tr("Change the number of frames"), this);
  // TODO: set a shortcut to changing the number of frames
  connect(change_num_frames_action, SIGNAL(triggered()),
          this, SLOT(change_num_frames_requested()));
  addAction(change_num_frames_action);

  change_num_columns_action = new QAction(
        tr("Change the number of columns"), this);
  // TODO: set a shortcut to changing the number of columns
  connect(change_num_columns_action, SIGNAL(triggered()),
          this, SLOT(change_num_columns_requested()));
  addAction(change_num_columns_action);

  ViewSettings* view_settings = new ViewSettings(this);
  set_view_settings(*view_settings);
}

/**
 * @brief Returns the sprite scene represented in this view.
 * @return The scene or nullptr if no sprite was set.
 */
SpriteScene* SpriteView::get_scene() {
  return scene;
}

/**
 * @brief Sets the sprite to represent in this view.
 * @param model The sprite model, or nullptr to remove any model.
 * This class does not take ownership on the model.
 * The model can be deleted safely.
 */
void SpriteView::set_model(SpriteModel* model) {

  if (this->model != nullptr) {
    this->model = nullptr;
    this->scene = nullptr;
  }

  this->model = model;

  if (model != nullptr) {
    // Create the scene from the model.
    scene = new SpriteScene(*model, this);
    setScene(scene);

    // Enable useful features if there is an image.
    setDragMode(QGraphicsView::RubberBandDrag);

    if (view_settings != nullptr) {
      view_settings->set_zoom(2.0);  // Initial zoom: x2.
    }
    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);

    // Install panning and zooming helpers.
    new PanTool(this);
    new ZoomTool(this);
  }
}

/**
 * @brief Sets the view settings for this view.
 *
 * When they change, the view is updated accordingly.
 *
 * @param view_settings The settings to watch.
 */
void SpriteView::set_view_settings(ViewSettings& view_settings) {

  this->view_settings = &view_settings;

  connect(&view_settings, SIGNAL(zoom_changed(double)),
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

  horizontalScrollBar()->setValue(0);
  verticalScrollBar()->setValue(0);
}

/**
 * @brief Sets the zoom level of the view from the settings.
 *
 * Zooming will be anchored at the mouse position.
 * The zoom value will be clamped between 0.25 and 4.0.
 */
void SpriteView::update_zoom() {

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
void SpriteView::zoom_in() {

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
void SpriteView::zoom_out() {

  if (view_settings == nullptr) {
    return;
  }

  view_settings->set_zoom(view_settings->get_zoom() / 2.0);
}

/**
 * @brief Shows or hides the grid according to the view settings.
 */
void SpriteView::update_grid_visibility() {

  if (view_settings == nullptr) {
    return;
  }

  if (view_settings->is_grid_visible()) {
    // Necessary to correctly show the grid when scrolling,
    // because it is part of the foreground, not of graphics items.
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  }
  else {
    // Faster.
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
  }

  if (scene != nullptr) {
    // The foreground has changed.
    scene->invalidate();
  }
}

/**
 * @brief Slot called when the user asks for ducplicate the selected direction.
 */
void SpriteView::duplicate_selected_direction_requested() {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_direction_index()) {
    return;
  }
  QPoint posistion = model->get_direction_position(index);
  emit duplicate_selected_direction_requested(posistion);
}

/**
 * @brief Maps a mouse event position to a scene position.
 * @param point The mouse position.
 * @param snap_to_grid Set to @c true to snap the position to the grid.
 * @return The scene position.
 */
QPoint SpriteView::map_to_scene(const QPoint& point, bool snap_to_grid) {

  QPoint mapped_point = mapToScene(point).toPoint();

  if (snap_to_grid) {
    // TODO: use the grid size (settings).
    mapped_point.setX(qFloor(mapped_point.x() / 8) * 8);
    mapped_point.setY(qFloor(mapped_point.y() / 8) * 8);
  }

  return mapped_point;
}

/**
 * @brief Change the number of frames and columns of the selected direction.
 * @param mode The changing mode.
 */
void SpriteView::change_num_frames_columns(
  const ChangingNumFramesColumnsMode& mode) {

  if (state != State::NORMAL) {
    return;
  }

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_direction_index()) {
    return;
  }

  start_state_changing_num_frames_columns(mode);
}

/**
 * @brief Slot called when the user asks for change number of frames/columns.
 */
void SpriteView::change_num_frames_columns_requested() {

  change_num_frames_columns(ChangingNumFramesColumnsMode::CHANGE_BOTH);
}

/**
 * @brief Slot called when the user asks for change number of frames.
 */
void SpriteView::change_num_frames_requested() {

  change_num_frames_columns(ChangingNumFramesColumnsMode::CHANGE_NUM_FRAMES);
}

/**
 * @brief Slot called when the user asks for change number of columns.
 */
void SpriteView::change_num_columns_requested() {

  change_num_frames_columns(ChangingNumFramesColumnsMode::CHANGE_NUM_COLUMNS);
}

/**
 * @brief Draws the sprite view.
 * @param event The paint event.
 */
void SpriteView::paintEvent(QPaintEvent* event) {

  QGraphicsView::paintEvent(event);

  if (view_settings == nullptr || !view_settings->is_grid_visible()) {
    return;
  }

  QSize grid = view_settings->get_grid_size();
  QRect rect = event->rect();
  rect.setTopLeft(mapFromScene(0, 0));

  // Draw the grid.
  QPainter painter(viewport());
  GuiTools::draw_grid(
    painter, rect, grid * zoom, view_settings->get_grid_color(),
    view_settings->get_grid_style());
}

/**
 * @brief Receives a focus out event.
 * @param event The event to handle.
 */
void SpriteView::focusOutEvent(QFocusEvent* event) {

  if (state == State::CHANGING_NUM_FRAMES_COLUMNS) {
    cancel_state_changing_num_frames_columns();
  }
  QGraphicsView::focusOutEvent(event);
}

/**
 * @brief Receives a key press event.
 * @param event The event to handle.
 */
void SpriteView::keyPressEvent(QKeyEvent* event) {

  if (event->key() == Qt::Key_Escape &&
      state == State::CHANGING_NUM_FRAMES_COLUMNS) {
    cancel_state_changing_num_frames_columns();
  }
  QGraphicsView::keyPressEvent(event);
}

/**
 * @brief Receives a mouse press event.
 *
 * Reimplemented to handle the selection.
 *
 * @param event The event to handle.
 */
void SpriteView::mousePressEvent(QMouseEvent* event) {

  if (model == nullptr || state == State::CHANGING_NUM_FRAMES_COLUMNS) {
    return;
  }

  if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {

    // Left or right button: possibly change the selection.
    QList<QGraphicsItem*> items_under_mouse = items(
          QRect(event->pos(), QSize(1, 1)),
          Qt::IntersectsItemBoundingRect  // Pick transparent items too.
          );
    QGraphicsItem* item = items_under_mouse.empty() ? nullptr : items_under_mouse.first();

    if (item != nullptr) {
      if (!item->isSelected()) {
        // Select the item.
        scene->clearSelection();
        item->setSelected(true);
      }
      if (event->button() == Qt::LeftButton &&
               model->get_selected_index().is_direction_index()) {
        // Allow to move it.
        start_state_moving_direction(event->pos());
      }
    }
    else {
      // Left click outside items: trace a selection rectangle.
      scene->clearSelection();
      start_state_drawing_rectangle(event->pos());
    }
  }
}

/**
 * @brief Receives a mouse release event.
 *
 * Reimplemented to scroll the view when the middle mouse button is pressed.
 *
 * @param event The event to handle.
 */
void SpriteView::mouseReleaseEvent(QMouseEvent* event) {

  if (model == nullptr) {
    return;
  }

  if (state == State::DRAWING_RECTANGLE) {
    end_state_drawing_rectangle();
  }
  else if (state == State::MOVING_DIRECTION) {
    end_state_moving_direction();
  }
  else if (state == State::CHANGING_NUM_FRAMES_COLUMNS) {
    end_state_changing_num_frames_columns();
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
void SpriteView::mouseMoveEvent(QMouseEvent* event) {

  if (model == nullptr) {
    return;
  }

  bool update_selection_validity = false;

  if (state == State::DRAWING_RECTANGLE) {

    // Compute the selected area.
    QPoint dragging_previous_point = dragging_current_point;
    dragging_current_point = map_to_scene(event->pos());

    if (dragging_current_point != dragging_previous_point) {

      int x = qMin(dragging_current_point.x(), dragging_start_point.x());
      int y = qMin(dragging_current_point.y(), dragging_start_point.y());
      int width = qAbs(dragging_current_point.x() - dragging_start_point.x());
      int height = qAbs(dragging_current_point.y() - dragging_start_point.y());

      // TODO: use the grid size (settings).
      if (x == dragging_start_point.x()) {
        width += 8;
      }

      if (y == dragging_start_point.y()) {
        height += 8;
      }

      current_area_item.setPos(QPoint(x, y));
      current_area_item.set_frame_size(QSize(width, height));
      update_selection_validity = true;
    }
  }
  else if (state == State::MOVING_DIRECTION) {

    SpriteModel::Index index = model->get_selected_index();
    if (!index.is_direction_index()) {
      // Direction was deselected: cancel the movement.
      end_state_moving_direction();
    }
    else {
      QPoint position = model->get_direction_position(index);
      QRect previous_rect = current_area_item.get_direction_all_frames_rect();

      dragging_current_point = map_to_scene(event->pos());
      current_area_item.setPos(QPoint(
        position.x() + dragging_current_point.x() - dragging_start_point.x(),
        position.y() + dragging_current_point.y() - dragging_start_point.y()));
      update_selection_validity = true;

      // To ensure that the previous area is clean.
      scene->invalidate(previous_rect);
    }
  } else if (state == State::CHANGING_NUM_FRAMES_COLUMNS) {

    SpriteModel::Index index = model->get_selected_index();
    if (!index.is_direction_index()) {
      cancel_state_changing_num_frames_columns();
    }
    else {
      int num_frames = 1;
      int num_columns = 1;

      dragging_current_point = map_to_scene(event->pos(), false);
      compute_num_frames_columns(num_frames, num_columns);

      current_area_item.set_num_frames(num_frames);
      current_area_item.set_num_columns(num_columns);
      update_selection_validity = true;
    }
  }

  if (update_selection_validity) {
    QRect rect = current_area_item.get_direction_all_frames_rect();
    current_area_item.set_valid(!rect.isEmpty() && sceneRect().contains(rect));
  }

  // The parent class tracks mouse movements for internal needs
  // such as anchoring the viewport to the mouse when zooming.
  QGraphicsView::mouseMoveEvent(event);
}

/**
 * @brief Receives a context menu event.
 * @param event The event to handle.
 */
void SpriteView::contextMenuEvent(QContextMenuEvent* event) {

  if (scene == nullptr) {
    return;
  }

  QPoint where;
  if (event->pos() != QPoint(0, 0)) {
    where = event->pos();
  }
  else {
    QList<QGraphicsItem*> selected_items = scene->selectedItems();
    where = mapFromScene(selected_items.first()->pos() + QPoint(8, 8));
  }

  show_context_menu(where);
}

/**
 * @brief Shows a context menu with actions relative to the selected directions.
 *
 * Does nothing if the view is in read-only mode.
 *
 * @param where Where to show the menu, in view coordinates.
 */
void SpriteView::show_context_menu(const QPoint& where) {

  if (model == nullptr) {
    return;
  }

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_direction_index()) {
    return;
  }

  QMenu* menu = new QMenu(this);

  // Delete direction.
  menu->addAction(duplicate_direction_action);
  menu->addSeparator();
  menu->addAction(change_num_frames_columns_action);
  menu->addAction(change_num_frames_action);
  menu->addAction(change_num_columns_action);
  menu->addSeparator();
  menu->addAction(delete_direction_action);

  // Create the menu at 1,1 to avoid the cursor being already in the first item.
  menu->popup(viewport()->mapToGlobal(where) + QPoint(1, 1));
}

/**
 * @brief Sets the normal state.
 */
void SpriteView::start_state_normal() {

  this->state = State::NORMAL;
}

/**
 * @brief Moves to the state of drawing a rectangle for a selection or a
 * new direction.
 * @param initial_point Where the user starts drawing the rectangle,
 * in view coordinates.
 */
void SpriteView::start_state_drawing_rectangle(const QPoint& initial_point) {

  state = State::DRAWING_RECTANGLE;
  dragging_start_point = map_to_scene(initial_point);
  dragging_current_point = dragging_start_point;

  current_area_item.setPos(dragging_current_point);
  current_area_item.set_frame_size(QSize(8, 8));
  current_area_item.set_num_frames(1);
  current_area_item.set_num_columns(1);
  current_area_item.set_valid(true);
  scene->addItem(&current_area_item);
}

/**
 * @brief Finishes drawing a rectangle.
 */
void SpriteView::end_state_drawing_rectangle() {

  QRect rectangle = current_area_item.get_direction_all_frames_rect();
  if (!rectangle.isEmpty() &&
      sceneRect().contains(rectangle) &&
      !model->get_selected_index().is_direction_index()) {

    // Context menu to create a direction.
    QMenu menu;
    QAction* new_direction_action = new QAction(tr("New direction"), this);
    connect(new_direction_action, &QAction::triggered, [this, rectangle] {
      emit add_direction_requested(rectangle);
    });
    menu.addAction(new_direction_action);
    menu.addSeparator();
    menu.addAction(tr("Cancel"));
    menu.exec(cursor().pos() + QPoint(1, 1));
  }

  scene->removeItem(&current_area_item);
  start_state_normal();
}

/**
 * @brief Moves to the state of moving the selected direction.
 * @param initial_point Where the user starts dragging the direction,
 * in view coordinates.
 */
void SpriteView::start_state_moving_direction(const QPoint& initial_point) {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_direction_index()) {
    return;
  }

  state = State::MOVING_DIRECTION;
  dragging_start_point = map_to_scene(initial_point);
  dragging_current_point = dragging_start_point;

  current_area_item.setPos(model->get_direction_position(index));
  current_area_item.set_frame_size(model->get_direction_size(index));
  current_area_item.set_num_frames(model->get_direction_num_frames(index));
  current_area_item.set_num_columns(model->get_direction_num_columns(index));
  current_area_item.set_valid(true);
  scene->addItem(&current_area_item);
}

/**
 * @brief Finishes moving a direction.
 */
void SpriteView::end_state_moving_direction() {

  SpriteModel::Index index = model->get_selected_index();
  QRect box = current_area_item.get_direction_all_frames_rect();
  if (!box.isEmpty() &&
      sceneRect().contains(box) &&
      index.is_direction_index() &&
      box != model->get_direction_all_frames_rect(index)) {

    // Context menu to move the direction.
    QMenu menu;
    QAction* move_direction_action = new QAction(tr("Move here"), this);
    connect(move_direction_action, &QAction::triggered, [this, box] {
      emit change_selected_direction_position_requested(box.topLeft());
    });
    menu.addAction(move_direction_action);
    QAction* duplicate_direction_action =
      new QAction(QIcon(":/images/icon_copy.png"), tr("Duplicate here"), this);
    connect(duplicate_direction_action, &QAction::triggered, [this, box] {
      emit duplicate_selected_direction_requested(box.topLeft());
    });
    menu.addAction(duplicate_direction_action);
    menu.addSeparator();
    menu.addAction(tr("Cancel"));
    menu.exec(cursor().pos() + QPoint(1, 1));
  }

  scene->removeItem(&current_area_item);
  start_state_normal();
}

/**
 * @brief Moves to the state of changing the number of frames and columns.
 * @param mode The changing mode.
 */
void SpriteView::start_state_changing_num_frames_columns(
  const ChangingNumFramesColumnsMode& mode) {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_direction_index()) {
    return;
  }

  state = State::CHANGING_NUM_FRAMES_COLUMNS;
  changing_mode = mode;

  current_area_item.setPos(model->get_direction_position(index));
  current_area_item.set_frame_size(model->get_direction_size(index));
  current_area_item.set_num_frames(model->get_direction_num_frames(index));
  current_area_item.set_num_columns(model->get_direction_num_columns(index));
  current_area_item.set_valid(true);
  scene->addItem(&current_area_item);
}

/**
 * @brief Finishes changing the number of frames and columns.
 */
void SpriteView::end_state_changing_num_frames_columns() {

  SpriteModel::Index index = model->get_selected_index();
  if (index.is_direction_index()) {

    int num_frames = 1;
    int num_columns = 1;

    compute_num_frames_columns(num_frames, num_columns);

    emit change_direction_num_frames_columns_requested(num_frames, num_columns);
  }

  cancel_state_changing_num_frames_columns();
}

/**
 * @brief Cancels changing the number of frames and columns.
 */
void SpriteView::cancel_state_changing_num_frames_columns() {

  scene->removeItem(&current_area_item);
  start_state_normal();
}

/**
 * @brief Computes the current number of frames and columns for changing state.
 * @param[ou] num_frames The number of frame.
 * @param[ou] num_columns The number of columns.
 */
void SpriteView::compute_num_frames_columns(int& num_frames, int& num_columns) {

  SpriteModel::Index index = model->get_selected_index();
  if (!index.is_direction_index()) {
    return;
  }

  QRect rect = model->get_direction_first_frame_rect(index);

  if (changing_mode == ChangingNumFramesColumnsMode::CHANGE_BOTH) {

    int x = dragging_current_point.x() + rect.width() - rect.x();
    x = (x / rect.width()) * rect.width();
    x = qMax(x, rect.width());

    int y = dragging_current_point.y() + rect.height() - rect.y();
    y = (y / rect.height()) * rect.height();
    y = qMax(y, rect.height());

    num_columns = x / rect.width();
    num_frames = (y / rect.height()) * num_columns;
  }
  else if (changing_mode == ChangingNumFramesColumnsMode::CHANGE_NUM_FRAMES) {
    // TODO: implement this mode.
  }
  else if (changing_mode == ChangingNumFramesColumnsMode::CHANGE_NUM_COLUMNS) {
    // TODO: implement this mode.
  }
}

/**
 * @brief Creates a selection item.
 */
SpriteView::DirectionAreaItem::DirectionAreaItem() :
  frame_size(8, 8),
  num_frames(1),
  num_columns(1),
  is_valid(true) {
  update_bouding_rect();
}

/**
 * @brief Changes the size of the frames.
 * @param size The size.
 */
void SpriteView::DirectionAreaItem::set_frame_size(const QSize& size) {

  frame_size = size;
  update_bouding_rect();
}

/**
 * @brief Change the number of frames.
 * @param num_frames The number of frames.
 */
void SpriteView::DirectionAreaItem::set_num_frames(int num_frames) {

  this->num_frames = qMax(num_frames, 1);
  update_bouding_rect();
}

/**
 * @brief Changes the number of columns.
 * @param num_columns The number of columns.
 */
void SpriteView::DirectionAreaItem::set_num_columns(int num_columns) {

  this->num_columns = qMax(num_columns, 1);
  update_bouding_rect();
}

/**
 * @brief Changes whether the area is valid.
 * @param valid Whether the area is valid.
 */
void SpriteView::DirectionAreaItem::set_valid(bool valid) {

  is_valid = valid;
}

/**
 * @brief Returns a rect that contains all frames of a direction.
 * @return The direction's frames rect.
 */
QRect SpriteView::DirectionAreaItem::get_direction_all_frames_rect() {

  QRectF rectf = boundingRect();
  rectf.translate(pos());
  return rectf.toRect();
}

/**
 * @brief Returns the bounding rect.
 * @return The bouding rect.
 */
QRectF SpriteView::DirectionAreaItem::boundingRect() const {

  return bounding_rect;
}

/**
 * @brief The paint event.
 */
void SpriteView::DirectionAreaItem::paint(
  QPainter* painter, const QStyleOptionGraphicsItem* /* option */,
  QWidget* /* widget */) {

  QSize draw_size = frame_size - QSize(1, 1);

  painter->save();
  painter->setPen(is_valid ? Qt::yellow : Qt::red);

  for (int i = 0; i < num_frames; ++i) {
    int row = qFloor(i / num_columns);
    int column = i % num_columns;
    QPoint pos = QPoint(frame_size.width() * column, frame_size.height() * row);
    painter->drawRect(QRect(pos, draw_size));
  }

  painter->restore();
}

/**
 * @brief Updates the bounding rect.
 */
void SpriteView::DirectionAreaItem::update_bouding_rect() {

  int num_columns = qMin(this->num_columns, num_frames);
  int num_rows = qFloor((num_frames - 1) / num_columns) + 1;

  prepareGeometryChange();
  bounding_rect = QRect(
    0, 0, frame_size.width() * num_columns, frame_size.height() * num_rows);
}

}
