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
#include "gui/enum_menus.h"
#include "gui/gui_tools.h"
#include "gui/tileset_scene.h"
#include "gui/tileset_view.h"
#include "ground_traits.h"
#include "pattern_animation_traits.h"
#include "pattern_separation_traits.h"
#include "tileset_model.h"
#include <QAction>
#include <QApplication>
#include <QGraphicsItem>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>

/**
 * @brief Creates a tileset view.
 * @param parent The parent widget or nullptr.
 */
TilesetView::TilesetView(QWidget* parent) :
  QGraphicsView(parent),
  scene(nullptr),
  change_pattern_id_action(nullptr),
  delete_patterns_action(nullptr),
  last_integer_pattern_id(0),
  state(State::NORMAL),
  current_area_item(nullptr),
  zoom(1.0),
  read_only(false) {

  setAlignment(Qt::AlignTop | Qt::AlignLeft);

  change_pattern_id_action = new QAction(
        QIcon(":/images/icon_edit.png"), tr("Change id..."), this);
  change_pattern_id_action->setShortcut(tr("F2"));
  change_pattern_id_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(change_pattern_id_action, SIGNAL(triggered()),
          this, SIGNAL(change_selected_pattern_id_requested()));
  addAction(change_pattern_id_action);

  delete_patterns_action = new QAction(
        QIcon(":/images/icon_delete.png"), tr("Delete..."), this);
  delete_patterns_action->setShortcut(QKeySequence::Delete);
  delete_patterns_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(delete_patterns_action, SIGNAL(triggered()),
          this, SIGNAL(delete_selected_patterns_requested()));
  addAction(delete_patterns_action);
}

/**
 * @brief Sets the tileset to represent in this view.
 * @param model The tileset model, or nullptr to remove any model.
 * This class does not take ownership on the model.
 * The model can be deleted safely.
 */
void TilesetView::set_model(TilesetModel* model) {

  if (this->model != nullptr) {
    this->model = nullptr;
    this->scene = nullptr;
  }

  this->model = model;

  if (model != nullptr) {
    // Create the scene from the model.
    scene = new TilesetScene(*model, this);
    setScene(scene);

    if (model->get_patterns_image().isNull()) {
      return;
    }

    // Enable useful features if there is an image.
    setDragMode(QGraphicsView::RubberBandDrag);
    set_zoom(2.0);  // Initial zoom: x2.
    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);
  }
}

/**
 * @brief Returns the zoom level of the view.
 * @return The zoom level.
 */
double TilesetView::get_zoom() const {
  return zoom;
}

/**
 * @brief Returns whether the view is in read-only mode.
 * @return @c true if the mode is read-only, @c false if changes can be
 * made to the tileset.
 */
bool TilesetView::is_read_only() const {
  return read_only;
}

/**
 * @brief Sets whether the view is in read-only mode.
 * @param read_only @c true to block changes from this view, @c false to allow them.
 */
void TilesetView::set_read_only(bool read_only) {
  this->read_only = read_only;
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

  emit zoom_changed(zoom);
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

  if (model == nullptr) {
    return;
  }

  if (event->button() == Qt::MidButton) {
    // Middle button: pan the view.
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    pan_initial_point = QPoint(
          horizontalScrollBar()->value() + event->x(),
          verticalScrollBar()->value() + event->y()
          );
  }
  else if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {

    // Left or right button: possibly change the selection.
    QList<QGraphicsItem*> items_under_mouse = items(
          QRect(event->pos(), QSize(1, 1)),
          Qt::IntersectsItemBoundingRect  // Pick transparent items too.
          );
    QGraphicsItem* item = items_under_mouse.empty() ? nullptr : items_under_mouse.first();

    bool control_or_shift = (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier));

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
      scene->clearSelection();
    }

    if (event->button() == Qt::LeftButton) {

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
          // Allow to move it.
          if (model->get_selection_count() == 1 &&
              !is_read_only()) {
            start_state_moving_pattern(event->pos());
          }
        }
      }
      else {
        // Left click outside items: trace a selection rectangle.
        start_state_drawing_rectangle(event->pos());
      }
    }

    else if (event->button() == Qt::RightButton) {

      if (item != nullptr) {
        if (!item->isSelected()) {
          // Select the right-clicked item.
          item->setSelected(true);
        }
      }
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
void TilesetView::mouseReleaseEvent(QMouseEvent* event) {

  if (model == nullptr) {
    return;
  }

  if (event->button() == Qt::MidButton) {
    QApplication::restoreOverrideCursor();
    return;
  }

  if (state == State::DRAWING_RECTANGLE) {
    end_state_drawing_rectangle();
  }
  else if (state == State::MOVING_PATTERN) {
    end_state_moving_pattern();
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

  if (model == nullptr) {
    return;
  }

  if ((event->buttons() & Qt::MidButton) == Qt::MidButton) {

    QPoint scroll_point(
          pan_initial_point.x() - event->x(),
          pan_initial_point.y() - event->y()
    );
    horizontalScrollBar()->setValue(scroll_point.x());
    verticalScrollBar()->setValue(scroll_point.y());
    return;
  }

  if (state == State::DRAWING_RECTANGLE) {

    // Compute the selected area.
    QPoint dragging_previous_point = dragging_current_point;
    dragging_current_point = mapToScene(event->pos()).toPoint() / 8 * 8;

    if (dragging_current_point != dragging_previous_point) {

      QRect new_pattern_area;

      // The area has changed: recalculate the rectangle.
      if (dragging_start_point.x() < dragging_current_point.x()) {
        new_pattern_area.setX(dragging_start_point.x());
        new_pattern_area.setWidth(dragging_current_point.x() - dragging_start_point.x());
      }
      else {
        new_pattern_area.setX(dragging_current_point.x());
        new_pattern_area.setWidth(dragging_start_point.x() - dragging_current_point.x());
      }

      if (dragging_start_point.y() < dragging_current_point.y()) {
        new_pattern_area.setY(dragging_start_point.y());
        new_pattern_area.setHeight(dragging_current_point.y() - dragging_start_point.y());
      }
      else {
        new_pattern_area.setY(dragging_current_point.y());
        new_pattern_area.setHeight(dragging_start_point.y() - dragging_current_point.y());
      }

      set_current_area(new_pattern_area);
    }
  }
  else if (state == State::MOVING_PATTERN) {

    int index = model->get_selected_index();
    if (index == -1) {
      // Tile was deselected: cancel the movement.
      end_state_moving_pattern();
    }
    else {
      dragging_current_point = mapToScene(event->pos()).toPoint() / 8 * 8;

      QRect new_pattern_area = current_area_item->rect().toRect();
      QRect old_pattern_area = model->get_pattern_frame(index);
      new_pattern_area.moveTopLeft(QPoint(
            old_pattern_area.x() + dragging_current_point.x() - dragging_start_point.x(),
            old_pattern_area.y() + dragging_current_point.y() - dragging_start_point.y()));

      set_current_area(new_pattern_area);
    }
  }

  QGraphicsView::mouseMoveEvent(event);
}

/**
 * @brief Receives a mouse wheel event.
 * @param event The event to handle.
 */
void TilesetView::wheelEvent(QWheelEvent* event) {

  if (model == nullptr) {
    return;
  }

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

/**
 * @brief Receives a context menu event.
 * @param event The event to handle.
 */
void TilesetView::contextMenuEvent(QContextMenuEvent* event) {

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
 * @brief Shows a context menu with actions relative to the selected patterns.
 *
 * Does nothing if the view is in read-only mode.
 *
 * @param where Where to show the menu, in view coordinates.
 */
void TilesetView::show_context_menu(const QPoint& where) {

  if (model == nullptr) {
    return;
  }

  if (is_read_only()) {
    return;
  }

  QList<int> selected_indexes = model->get_selected_indexes();
  if (selected_indexes.empty()) {
    return;
  }

  QMenu* menu = new QMenu(this);

  // Ground.
  build_context_menu_ground(*menu, selected_indexes);

  // Default layer.
  QMenu* layer_menu = new QMenu(tr("Default layer"), this);
  build_context_menu_layer(*layer_menu, selected_indexes);
  menu->addSeparator();
  menu->addMenu(layer_menu);

  // Animation.
  QMenu* animation_menu = new QMenu(tr("Animation"), this);
  build_context_menu_animation(*animation_menu, selected_indexes);
  menu->addSeparator();
  menu->addMenu(animation_menu);

  // Change pattern id.
  menu->addSeparator();
  change_pattern_id_action->setEnabled(model->get_selected_index() != -1);
  menu->addAction(change_pattern_id_action);

  // Delete patterns.
  menu->addSeparator();
  menu->addAction(delete_patterns_action);

  // Create the menu at 1,1 to avoid the cursor being already in the first item.
  menu->popup(viewport()->mapToGlobal(where) + QPoint(1, 1));
}

/**
 * @brief Builds the ground part of a context menu for patterns.
 * @param menu The menu to fill.
 * @param indexes Patterns to build a context menu for.
 */
void TilesetView::build_context_menu_ground(
    QMenu& menu, const QList<int>& indexes) {

  if (indexes.empty()) {
    return;
  }

  // See if the ground is common.
  Ground ground;
  bool common = model->is_common_pattern_ground(indexes, ground);

  // Add ground actions to the menu.
  QList<QAction*> ground_actions = EnumMenus<Ground>::create_actions(
        menu,
        EnumMenuCheckableOption::CHECKABLE_EXCLUSIVE,
        [=](Ground ground) {
    emit change_selected_patterns_ground_requested(ground);
  });

  if (common) {
    int ground_index = static_cast<int>(ground);
    QAction* checked_action = ground_actions[ground_index];
    checked_action->setChecked(true);
    // Add a checkmark (there is none when there is already an icon).
    checked_action->setText("\u2714 " + checked_action->text());
  }
}

/**
 * @brief Builds the default layer part of a context menu for patterns.
 * @param menu The menu to fill.
 * @param indexes Patterns to build a context menu for.
 */
void TilesetView::build_context_menu_layer(
    QMenu& menu, const QList<int>& indexes) {

  if (indexes.empty()) {
    return;
  }

  // See if the default layer is common.
  Layer layer;
  bool common = model->is_common_pattern_default_layer(indexes, layer);

  // Add layer actions to the menu.
  QList<QAction*> layer_actions = EnumMenus<Layer>::create_actions(
        menu,
        EnumMenuCheckableOption::CHECKABLE_EXCLUSIVE,
        [=](Layer layer) {
    emit change_selected_patterns_default_layer_requested(layer);
  });

  if (common) {
    int layer_index = static_cast<int>(layer);
    QAction* checked_action = layer_actions[layer_index];
    checked_action->setChecked(true);
  }
}

/**
 * @brief Builds the animation layer part of a context menu for patterns.
 * @param menu The menu to fill.
 * @param indexes Patterns to build a context menu for.
 */
void TilesetView::build_context_menu_animation(
    QMenu& menu, const QList<int>& indexes) {

  if (indexes.empty()) {
    return;
  }

  // See if the animation and the separation are common.
  PatternAnimation animation;
  PatternSeparation separation;
  bool common_animation = model->is_common_pattern_animation(indexes, animation);
  bool common_separation = model->is_common_pattern_separation(indexes, separation);
  bool enable_separation = common_animation &&
      PatternAnimationTraits::is_multi_frame(animation);

  // Add actions to the menu.
  QList<QAction*> animation_actions = EnumMenus<PatternAnimation>::create_actions(
        menu,
        EnumMenuCheckableOption::CHECKABLE_EXCLUSIVE,
        [=](PatternAnimation animation) {
    emit change_selected_patterns_animation_requested(animation);
  });
  menu.addSeparator();
  QList<QAction*> separation_actions = EnumMenus<PatternSeparation>::create_actions(
        menu,
        EnumMenuCheckableOption::CHECKABLE_EXCLUSIVE,
        [=](PatternSeparation separation) {
    emit change_selected_patterns_separation_requested(separation);
  });

  if (common_animation) {
    int animation_index = static_cast<int>(animation);
    QAction* checked_action = animation_actions[animation_index];
    checked_action->setChecked(true);
  }

  if (enable_separation) {
    if (common_separation) {
      int separation_index = static_cast<int>(separation);
      QAction* checked_action = separation_actions[separation_index];
      checked_action->setChecked(true);
      // Add a checkmark (there is none when there is already an icon).
      checked_action->setText("\u2714 " + checked_action->text());
    }
  }
  else {
    for (QAction* action : separation_actions) {
      action->setEnabled(false);
    }
  }
}

/**
 * @brief Sets the normal state.
 */
void TilesetView::start_state_normal() {

  this->state = State::NORMAL;
}

/**
 * @brief Moves to the state of drawing a rectangle for a selection or a
 * new pattern.
 * @param initial_point Where the user starts drawing the rectangle,
 * in view coordinates.
 */
void TilesetView::start_state_drawing_rectangle(const QPoint& initial_point) {

  this->state = State::DRAWING_RECTANGLE;
  this->dragging_start_point = mapToScene(initial_point).toPoint() / 8 * 8;

  current_area_item = new QGraphicsRectItem();
  current_area_item->setPen(QPen(Qt::yellow));
  scene->addItem(current_area_item);
}

/**
 * @brief Finishes drawing a rectangle.
 */
void TilesetView::end_state_drawing_rectangle() {

  QRect rectangle = current_area_item->rect().toRect();
  if (!rectangle.isEmpty() &&
      sceneRect().contains(rectangle) &&
      get_items_intersecting_current_area().isEmpty() &&
      model->is_selection_empty() &&
      !is_read_only()) {

    // Context menu to create a pattern.
    QMenu menu;
    EnumMenus<Ground>::create_actions(
          menu, EnumMenuCheckableOption::NON_CHECKABLE, [=](Ground ground) {
      QString pattern_id;
      do {
        ++last_integer_pattern_id;
        pattern_id = QString::number(last_integer_pattern_id);
      } while (model->id_to_index(pattern_id) != -1);

      emit create_pattern_requested(pattern_id, rectangle, ground);
    });

    // Put most actions in a submenu to make the context menu smaller.
    QMenu sub_menu(tr("New pattern (more options)"));
    int i = 0;
    for (QAction* action : menu.actions()) {
      Ground ground = static_cast<Ground>(action->data().toInt());
      if (ground == Ground::TRAVERSABLE ||
          ground == Ground::WALL) {
        action->setText(tr("New pattern (%1)").arg(GroundTraits::get_friendly_name(ground)));
      }
      else {
        menu.removeAction(action);
        sub_menu.addAction(action);
      }
      ++i;
    }
    menu.addMenu(&sub_menu);

    menu.addSeparator();
    menu.addAction(tr("Cancel"));
    menu.exec(cursor().pos() + QPoint(1, 1));
  }

  scene->removeItem(current_area_item);
  delete current_area_item;
  current_area_item = nullptr;

  start_state_normal();
}

/**
 * @brief Moves to the state of moving the selected pattern.
 * @param initial_point Where the user starts dragging the pattern,
 * in view coordinates.
 */
void TilesetView::start_state_moving_pattern(const QPoint& initial_point) {

  int index = model->get_selected_index();
  if (index == -1) {
    return;
  }

  state = State::MOVING_PATTERN;
  dragging_start_point = mapToScene(initial_point).toPoint()/ 8 * 8;
  const QRect& box = model->get_pattern_frames_bounding_box(index);
  current_area_item = new QGraphicsRectItem(box);
  current_area_item->setPen(QPen(Qt::yellow));
  scene->addItem(current_area_item);
}

/**
 * @brief Finishes moving a pattern.
 */
void TilesetView::end_state_moving_pattern() {

  QRect box = current_area_item->rect().toRect();
  if (!box.isEmpty() &&
      sceneRect().contains(box) &&
      get_items_intersecting_current_area().isEmpty() &&
      model->get_selection_count() == 1 &&
      !is_read_only()) {

    // Context menu to move the pattern.
    QMenu menu;
    QAction* move_pattern_action = new QAction(tr("Move here"), this);
    connect(move_pattern_action, &QAction::triggered, [=] {
      emit change_selected_pattern_position_requested(box.topLeft());
    });
    menu.addAction(move_pattern_action);
    menu.addSeparator();
    menu.addAction(tr("Cancel"));
    menu.exec(cursor().pos() + QPoint(1, 1));
  }

  scene->removeItem(current_area_item);
  delete current_area_item;
  current_area_item = nullptr;

  start_state_normal();
}

/**
 * @brief Changes the position of the pattern the user is creating or moving.
 *
 * If the specified area is the same as before, nothing is done.
 *
 * @param new_area new position of the pattern.
 */
void TilesetView::set_current_area(const QRect& area) {

  if (current_area_item->rect().toRect() == area) {
    // No change.
    return;
  }

  current_area_item->setRect(area);

  if (state == State::DRAWING_RECTANGLE) {
    // Select items strictly in the rectangle.
    scene->clearSelection();
    QPainterPath path;
    path.addRect(QRect(area.topLeft() - QPoint(1, 1),
                       area.size() + QSize(2, 2)));
    scene->setSelectionArea(path, Qt::ContainsItemBoundingRect);
  }
}

/**
 * @brief Returns all items fully contained in the rectangle drawn by the user.
 * @return The items in the drawn rectangle.
 */
QList<QGraphicsItem*> TilesetView::get_items_in_current_area() const {

  const QRect& area = current_area_item->rect().toRect();
  QRect outline(
      area.topLeft() - QPoint(1, 1),
      area.size() + QSize(2, 2));
  QList<QGraphicsItem*> items = scene->items(outline, Qt::ContainsItemBoundingRect);
  items.removeAll(current_area_item);  // Ignore the drawn rectangle itself.
  return items;
}

/**
 * @brief Returns all items that intersect the rectangle drawn by the user.
 * @return The items thet intersect the drawn rectangle.
 */
QList<QGraphicsItem*> TilesetView::get_items_intersecting_current_area() const {

  QRect area = current_area_item->rect().toRect();
  area = QRect(
      area.topLeft() + QPoint(1, 1),
      area.size() - QSize(2, 2));
  QList<QGraphicsItem*> items = scene->items(area, Qt::IntersectsItemBoundingRect);
  items.removeAll(current_area_item);  // Ignore the drawn rectangle itself.
  return items;
}
