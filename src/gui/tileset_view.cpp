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
#include "gui/enum_menus.h"
#include "gui/tileset_scene.h"
#include "gui/tileset_view.h"
#include "ground_traits.h"
#include "tile_pattern_animation_traits.h"
#include "tile_pattern_separation_traits.h"
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
  zoom(1.0) {

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
    // Middle button: pan the view.
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    pan_initial_point = QPoint(
          horizontalScrollBar()->value() + event->x(),
          verticalScrollBar()->value() + event->y()
          );
  }
  else if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {

    // Left or right button: possibly change the selection.
    QGraphicsItem* item = itemAt(event->pos());

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
      scene()->clearSelection();
    }

    if (event->button() == Qt::LeftButton) {

      if (item != nullptr) {

        if (control_or_shift) {
          // Left-clicking an item while pressing control or shift: toggle it.
          item->setSelected(!item->isSelected());
        }
        else if (!item->isSelected()) {
          // Select the item.
          item->setSelected(true);
        }
      }
      else {
        // Left click outside items: trace a selection rectangle.
        // TODO
      }
    }

    else if (event->button() == Qt::RightButton) {

      if (item != nullptr) {
        if (!item->isSelected()) {
          // Select the right-clicked item.
          item->setSelected(true);
        }
      }

      // Show a context menu if at least one item is selected.
      QList<QGraphicsItem*> selected_items = scene()->selectedItems();
      if (!selected_items.empty()) {
        show_context_menu(event->pos());
        return;
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

/**
 * @brief Shows a context menu with actions relative to the selected patterns.
 * @param where Where to show the menu, in view coordinates.
 */
void TilesetView::show_context_menu(const QPoint& where) {

  QList<int> selected_indexes = model->get_selected_indexes();
  if (selected_indexes.empty()) {
    return;
  }

  // TODO context menu with
  // - Ground
  // - Animation
  // - Animation separation
  // - Default layer
  // - Change id (except when multi-selection)
  // - Delete
  QMenu* menu = new QMenu(this);


  // Ground.
  bool common_ground = true;
  Ground ground = model->get_pattern_ground(selected_indexes.first());
  for (int index : selected_indexes) {
    if (model->get_pattern_ground(index) != ground) {
      common_ground = false;
      break;
    }
  }

  EnumMenus<Ground>::create_actions(menu, EnumMenuCheckableOption::CHECKABLE_EXCLUSIVE);
  if (common_ground) {
    int ground_index = static_cast<int>(ground);
    QAction* checked_action = menu->actions()[ground_index];
    checked_action->setChecked(true);
    // Add a checkmark (there is none when there is already an icon).
    checked_action->setText("\u2714 " + checked_action->text());
  }

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
