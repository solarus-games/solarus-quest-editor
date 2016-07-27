/*
 * Copyright (C) 2014-2016 Christopho, Solarus - http://www.solarus-games.org
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
#include "widgets/enum_menus.h"
#include "widgets/gui_tools.h"
#include "widgets/pan_tool.h"
#include "widgets/tileset_scene.h"
#include "widgets/tileset_view.h"
#include "widgets/zoom_tool.h"
#include "ground_traits.h"
#include "pattern_animation_traits.h"
#include "pattern_separation_traits.h"
#include "rectangle.h"
#include "tileset_model.h"
#include "view_settings.h"
#include <QAction>
#include <QApplication>
#include <QGraphicsItem>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>

namespace SolarusEditor {

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
  view_settings(nullptr),
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

  set_repeat_mode_actions = EnumMenus<TilePatternRepeatMode>::create_actions(
        *this,
        EnumMenuCheckableOption::CHECKABLE_EXCLUSIVE,
        [this](TilePatternRepeatMode repeat_mode) {
    emit change_selected_patterns_repeat_mode_requested(repeat_mode);
  });
  // TODO add shortcut support to EnumMenus
  set_repeat_mode_actions[static_cast<int>(TilePatternRepeatMode::ALL)]->setShortcut(tr("R"));
  set_repeat_mode_actions[static_cast<int>(TilePatternRepeatMode::HORIZONTAL)]->setShortcut(tr("H"));
  set_repeat_mode_actions[static_cast<int>(TilePatternRepeatMode::VERTICAL)]->setShortcut(tr("V"));
  set_repeat_mode_actions[static_cast<int>(TilePatternRepeatMode::NONE)]->setShortcut(tr("N"));
  for (QAction* action : set_repeat_mode_actions) {
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  }

  ViewSettings* view_settings = new ViewSettings(this);
  set_view_settings(*view_settings);
}

/**
 * @brief Returns the tileset represented in this view.
 * @return The tileset, or nullptr if there is currently no tileset.
 */
TilesetModel* TilesetView::get_model() {

  return this->model;
}

/**
 * @brief Sets the tileset to represent in this view.
 * @param model The tileset model, or nullptr to remove any model.
 * This class does not take ownership on the model.
 */
void TilesetView::set_model(TilesetModel* model) {

  int horizontal_scrollbar_value = 0;
  int vertical_scrollbar_value = 0;
  double zoom = 2.0;  // Initial zoom: x2.

  if (this->model != nullptr) {
    this->model = nullptr;
    this->scene = nullptr;
    horizontal_scrollbar_value = horizontalScrollBar()->value();
    vertical_scrollbar_value = verticalScrollBar()->value();
    if (view_settings != nullptr) {
      zoom = view_settings->get_zoom();
    }
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
    if (view_settings != nullptr) {
      view_settings->set_zoom(zoom);  // Initial zoom: x2.
    }
    horizontalScrollBar()->setValue(horizontal_scrollbar_value);
    verticalScrollBar()->setValue(vertical_scrollbar_value);

    // Install panning and zooming helpers.
    new PanTool(this);
    new ZoomTool(this);
  }
}

/**
 * @brief Returns the tileset scene represented in this view.
 * @return The scene or nullptr if no tileset was set.
 */
TilesetScene* TilesetView::get_scene() {
  return scene;
}

/**
 * @brief Sets the view settings for this view.
 *
 * When they change, the view is updated accordingly.
 *
 * @param view_settings The settings to watch.
 */
void TilesetView::set_view_settings(ViewSettings& view_settings) {

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
 * @brief Sets the zoom level of the view from the settings.
 *
 * Zooming will be anchored at the mouse position.
 * The zoom value will be clamped between 0.25 and 4.0.
 */
void TilesetView::update_zoom() {

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
void TilesetView::zoom_in() {

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
void TilesetView::zoom_out() {

  if (view_settings == nullptr) {
    return;
  }

  view_settings->set_zoom(view_settings->get_zoom() / 2.0);
}

/**
 * @brief Shows or hides the grid according to the view settings.
 */
void TilesetView::update_grid_visibility() {

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
 * @brief Selects all patterns.
 */
void TilesetView::select_all() {

  if (scene == nullptr) {
    return;
  }

  scene->select_all();
}

/**
 * @brief Unselects all patterns.
 */
void TilesetView::unselect_all() {

  if (scene == nullptr) {
    return;
  }

  scene->unselect_all();
}

/**
 * @brief Draws the tileset view.
 * @param event The paint event.
 */
void TilesetView::paintEvent(QPaintEvent* event) {

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
 * @brief Receives a mouse press event.
 * @param event The event to handle.
 */
void TilesetView::mousePressEvent(QMouseEvent* event) {

  if (model == nullptr) {
    return;
  }

  if (state == State::NORMAL) {

    QList<QGraphicsItem*> items_under_mouse = items(
          QRect(event->pos(), QSize(1, 1)),
          Qt::IntersectsItemBoundingRect  // Pick transparent items too.
    );
    QGraphicsItem* item = items_under_mouse.empty() ? nullptr : items_under_mouse.first();

    const bool control_or_shift = (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier));

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
      if (item != nullptr &&
          item->isSelected() &&
          model->get_selection_count() == 1 &&
          !is_read_only()) {
        // Clicking on an already selected item: allow to move it.
        start_state_moving_pattern(event->pos());
      }
      else {
        // Otherwise initialize a selection rectangle.
        initially_selected_items = scene->selectedItems();
        start_state_drawing_rectangle(event->pos());
      }
    }
    else {
      if (item != nullptr && !item->isSelected()) {
        // Select the right-clicked item.
        item->setSelected(true);
        emit selection_changed_by_user();
      }
    }
  }
}

/**
 * @brief Receives a mouse release event.
 * @param event The event to handle.
 */
void TilesetView::mouseReleaseEvent(QMouseEvent* event) {

  if (model == nullptr) {
    return;
  }

  bool do_selection = false;
  if (state == State::DRAWING_RECTANGLE) {
    // If the rectangle is empty, consider it was a click and not a drag.
    // In this case we simply select the clicked item.
    do_selection = current_area_item->rect().isEmpty();
    end_state_drawing_rectangle();
  }
  else if (state == State::MOVING_PATTERN) {
    end_state_moving_pattern();
  }

  if (do_selection) {
    if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {

      // Left or right button: possibly change the selection.
      QList<QGraphicsItem*> items_under_mouse = items(
            QRect(event->pos(), QSize(1, 1)),
            Qt::IntersectsItemBoundingRect  // Pick transparent items too.
            );
      QGraphicsItem* item = items_under_mouse.empty() ? nullptr : items_under_mouse.first();

      const bool control_or_shift = (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier));

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
        bool selection_was_empty = get_model()->is_selection_empty();
        scene->clearSelection();

        if (item == nullptr && selection_was_empty) {
          // The user clicked outside any item, to unselect everything.
          emit selection_changed_by_user();
        }
      }

      if (item != nullptr) {
        // Clicked an item.

        if (event->button() == Qt::LeftButton) {

          if (control_or_shift) {
            // Left-clicking an item while pressing control or shift: toggle it.
            item->setSelected(!item->isSelected());
            emit selection_changed_by_user();
          }
          else {
            if (!item->isSelected()) {
              // Select the item.
              item->setSelected(true);
              emit selection_changed_by_user();
            }
          }
        }
      }
    }
  }

  QGraphicsView::mouseReleaseEvent(event);
}

/**
 * @brief Receives a mouse double click event.
 * @param event The event to handle.
 */
void TilesetView::mouseDoubleClickEvent(QMouseEvent* event) {

  // Nothing special but we don't want the behavior from the parent class.
  Q_UNUSED(event);
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

  if (state == State::DRAWING_RECTANGLE) {

    // Compute the selected area.
    QPoint dragging_previous_point = dragging_current_point;
    dragging_current_point = mapToScene(event->pos()).toPoint() / 8 * 8;

    if (dragging_current_point != dragging_previous_point) {

      // The area has changed: recalculate the rectangle.
      QRect new_pattern_area = Rectangle::from_two_points(dragging_start_point, dragging_current_point);
      set_current_area(new_pattern_area);
      emit selection_changed_by_user();
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

  // The parent class tracks mouse movements for internal needs
  // such as anchoring the viewport to the mouse when zooming.
  QGraphicsView::mouseMoveEvent(event);
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

  // Repeat mode.
  QMenu* repeat_mode_menu = new QMenu(tr("Repeatable"), this);
  build_context_menu_repeat_mode(*repeat_mode_menu, selected_indexes);
  menu->addSeparator();
  menu->addMenu(repeat_mode_menu);

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
        [this](Ground ground) {
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
  int common_layer = 0;
  bool common = model->is_common_pattern_default_layer(indexes, common_layer);

  // Add 3 layer actions to the menu.
  // (If more layers are necessary, the user can still use the spinbox
  // in the patterns properties view.)
  for (int i = 0; i < 3; ++i) {
    QAction* action = new QAction(tr("Layer %1").arg(i), &menu);
    action->setCheckable(true);
    menu.addAction(action);
    connect(action, &QAction::triggered, [this, i]() {
      emit change_selected_patterns_default_layer_requested(i);
    });

    if (common && i == common_layer) {
      action->setChecked(true);
    }
  }

}

/**
 * @brief Builds the repeat mode part of a context menu for patterns.
 * @param menu The menu to fill.
 * @param indexes Patterns to build a context menu for.
 */
void TilesetView::build_context_menu_repeat_mode(
    QMenu& menu, const QList<int>& indexes) {

  if (indexes.empty()) {
    return;
  }

  // See if the repeat mode is common.
  TilePatternRepeatMode repeat_mode = TilePatternRepeatMode::ALL;
  bool common = model->is_common_pattern_repeat_mode(indexes, repeat_mode);

  menu.addActions(set_repeat_mode_actions);

  if (common) {
    int repeat_mode_index = static_cast<int>(repeat_mode);
    QAction* checked_action = set_repeat_mode_actions[repeat_mode_index];
    checked_action->setChecked(true);
  }
}

/**
 * @brief Builds the animation part of a context menu for patterns.
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
        [this](PatternAnimation animation) {
    emit change_selected_patterns_animation_requested(animation);
  });
  menu.addSeparator();
  QList<QAction*> separation_actions = EnumMenus<PatternSeparation>::create_actions(
        menu,
        EnumMenuCheckableOption::CHECKABLE_EXCLUSIVE,
        [this](PatternSeparation separation) {
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
  this->dragging_current_point = this->dragging_start_point;

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
          menu, EnumMenuCheckableOption::NON_CHECKABLE, [this, rectangle](Ground ground) {
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
    Q_FOREACH (QAction* action, menu.actions()) {
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
  initially_selected_items.clear();

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
  dragging_current_point = dragging_start_point;
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
      !is_read_only() &&
      dragging_current_point != dragging_start_point) {

    // Context menu to move the pattern.
    QMenu menu;
    QAction* move_pattern_action = new QAction(tr("Move here"), this);
    connect(move_pattern_action, &QAction::triggered, [this, box] {
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
 * @brief Changes the position of the rectangle the user is drawing or moving.
 *
 * If the specified area is the same as before, nothing is done.
 *
 * @param new_area new position of the rectangle.
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

    // Re-select items that were already selected if Ctrl or Shift was pressed.
    Q_FOREACH (QGraphicsItem* item, initially_selected_items) {
      item->setSelected(true);
    }
  }

  if (state == State::MOVING_PATTERN) {
    // Check overlapping existing patterns.
    if (!area.isEmpty() &&
        sceneRect().contains(area) &&
        get_items_intersecting_current_area().isEmpty() &&
        model->get_selection_count() == 1 &&
        !is_read_only()) {
      current_area_item->setPen(QPen(Qt::yellow));
    } else {
      current_area_item->setPen(QPen(Qt::red));
    }
  }
}

/**
 * @brief Returns all items that intersect the rectangle drawn by the user
 * except selected items.
 * @return The items that intersect the drawn rectangle.
 */
QList<QGraphicsItem*> TilesetView::get_items_intersecting_current_area() const {

  QRect area = current_area_item->rect().toRect();
  area = QRect(
      area.topLeft() + QPoint(1, 1),
      area.size() - QSize(2, 2));
  QList<QGraphicsItem*> items = scene->items(area, Qt::IntersectsItemBoundingRect);
  items.removeAll(current_area_item);  // Ignore the drawn rectangle itself.

  // Ignore selected items.
  Q_FOREACH (QGraphicsItem* item, scene->selectedItems()) {
    items.removeAll(item);
  }

  return items;
}

}
