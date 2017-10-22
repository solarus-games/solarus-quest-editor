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
#include <QDrag>
#include <QGraphicsItem>
#include <QMenu>
#include <QMimeData>
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
  view_settings(nullptr),
  zoom(1.0),
  read_only(false) {

  setAcceptDrops(true);
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
  set_repeat_mode_actions[static_cast<int>(TilePatternRepeatMode::ALL)]->setShortcut(tr("A"));
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

    // Restore the previous zoom and scrollbar positions.
    if (view_settings != nullptr) {
      view_settings->set_zoom(zoom);
    }

    horizontalScrollBar()->setValue(0);  // To force an actual change (refresh bug).
    horizontalScrollBar()->setValue(10);
    horizontalScrollBar()->setValue(horizontal_scrollbar_value);

    verticalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(10);
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
          !model->is_selection_empty() &&
          !control_or_shift &&
          !is_read_only()) {
        // Clicking on an already selected item: allow to move it.
        start_state_moving_patterns(event->pos());
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
    do_selection = current_area_items.first()->rect().isEmpty();
    end_state_drawing_rectangle();
  }
  else if (state == State::MOVING_PATTERNS) {
    end_state_moving_patterns();
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

      update_current_areas(dragging_start_point, dragging_current_point);
      emit selection_changed_by_user();
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

  QGraphicsRectItem *item = new QGraphicsRectItem();
  item->setPen(QPen(Qt::yellow));
  scene->addItem(item);
  current_area_items.push_front(item);
}

/**
 * @brief Finishes drawing a rectangle.
 */
void TilesetView::end_state_drawing_rectangle() {

  QRect rectangle = current_area_items.first()->rect().toRect();
  if (!rectangle.isEmpty() &&
      sceneRect().contains(rectangle) &&
      get_items_intersecting_current_areas().isEmpty() &&
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
    const QList<QAction*> actions = menu.actions();
    for (QAction* action : actions) {
      Ground ground = static_cast<Ground>(action->data().toInt());
      if (ground == Ground::TRAVERSABLE ||
          ground == Ground::WALL) {
        action->setText(tr("New pattern (%1)").arg(GroundTraits::get_friendly_name(ground)));
      }
      else {
        menu.removeAction(action);
        sub_menu.addAction(action);
      }
    }
    menu.addMenu(&sub_menu);

    menu.addSeparator();
    menu.addAction(tr("Cancel"));
    menu.exec(cursor().pos() + QPoint(1, 1));
  }

  clear_current_areas();
  initially_selected_items.clear();
  start_state_normal();
}

/**
 * @brief Moves to the state of moving the selected pattern.
 * @param initial_point Where the user starts dragging the pattern,
 * in view coordinates.
 */
void TilesetView::start_state_moving_patterns(const QPoint& initial_point) {

  if (model->is_selection_empty()) {
    return;
  }

  state = State::MOVING_PATTERNS;
  dragging_start_point = mapToScene(initial_point).toPoint() / 8 * 8;
  dragging_current_point = dragging_start_point;

  const QList<int>& selected_indexes = model->get_selected_indexes();
  for (int index : selected_indexes) {
    const QRect& box = model->get_pattern_frames_bounding_box(index);
    QGraphicsRectItem *item = new QGraphicsRectItem(box);
    item->setPen(QPen(Qt::yellow));
    scene->addItem(item);
    current_area_items.append(item);
  }

  const QRect& pattern_frame = model->get_pattern_frame(selected_indexes.first());
  const QPoint& hot_spot = initial_point - mapFromScene(pattern_frame.topLeft());
  QPixmap drag_pixmap = model->get_pattern_image(selected_indexes.first());

  if (view_settings != nullptr) {
    double zoom = view_settings->get_zoom();
    drag_pixmap = drag_pixmap.scaled(pattern_frame.size() * zoom);
  }

  // TODO make a pixmap of all selected patterns.
  QDrag* drag = new QDrag(this);
  drag->setPixmap(drag_pixmap);
  drag->setHotSpot(hot_spot);

  QStringList pattern_ids;
  for (int index : selected_indexes) {
    pattern_ids << model->index_to_id(index);
  }
  std::sort(pattern_ids.begin(), pattern_ids.end());
  QString text_data = pattern_ids.join("\n");

  QMimeData* data = new QMimeData();
  data->setText(text_data);

  drag->setMimeData(data);
  drag->exec(Qt::MoveAction | Qt::CopyAction);

  clear_current_areas();
  start_state_normal();
}

/**
 * @brief Finishes moving a pattern.
 */
void TilesetView::end_state_moving_patterns() {

  QPoint delta = dragging_current_point - dragging_start_point;
  QRect box = get_selection_bounding_box();
  box.translate(delta);
  if (!box.isEmpty() &&
      sceneRect().contains(box) &&
      get_items_intersecting_current_areas().isEmpty() &&
      !model->is_selection_empty() &&
      !is_read_only() &&
      dragging_current_point != dragging_start_point) {

    // Context menu to move the patterns.
    QMenu menu;
    QAction* move_pattern_action = new QAction(tr("Move here"), this);
    connect(move_pattern_action, &QAction::triggered, [this, delta] {
      emit change_selected_patterns_position_requested(delta);
    });
    menu.addAction(move_pattern_action);
    QAction* duplicate_pattern_action = new QAction(
      QIcon(":/images/icon_copy.png"), tr("Duplicate here"), this);
    duplicate_pattern_action->setEnabled(
      get_items_intersecting_current_areas(false).isEmpty());
    connect(duplicate_pattern_action, &QAction::triggered, [this, delta] {
      emit duplicate_selected_patterns_requested(delta);
    });
    menu.addAction(duplicate_pattern_action);
    menu.addSeparator();
    menu.addAction(tr("Cancel"));
    menu.exec(cursor().pos() + QPoint(1, 1));
  }

  clear_current_areas();
  start_state_normal();
}

void TilesetView::dragEnterEvent(QDragEnterEvent* event) {

  if (state != State::MOVING_PATTERNS) {
    return;
  }

  if (event->mimeData()->hasFormat("text/plain")) {
    event->acceptProposedAction();
  }
}

void TilesetView::dragMoveEvent(QDragMoveEvent* event) {

  if (state != State::MOVING_PATTERNS) {
    return;
  }

  dragging_current_point = mapToScene(event->pos()).toPoint() / 8 * 8;
  QPoint delta = dragging_current_point - dragging_start_point;

  clear_current_areas();

  bool valid_move = true;
  const QList<int>& selected_indexes = model->get_selected_indexes();
  for (int index : selected_indexes) {

    QRect area = model->get_pattern_frames_bounding_box(index);
    area.translate(delta);
    QGraphicsRectItem* item = new QGraphicsRectItem(area);

    // Check overlapping existing patterns.
    QList<QGraphicsItem*> items = scene->items(
      area.adjusted(1, 1, -1, -1), Qt::IntersectsItemBoundingRect);

    if (!area.isEmpty() &&
        sceneRect().contains(area) &&
        items.isEmpty() &&
        !is_read_only()) {
      item->setPen(QPen(Qt::yellow));
    } else {
      item->setPen(QPen(Qt::red));
      item->setZValue(1);
      valid_move = false;
    }

    // Let the drag cursor show if the move is legal.
    event->setAccepted(valid_move);

    scene->addItem(item);
    current_area_items.append(item);
  }
}

void TilesetView::dragLeaveEvent(QDragLeaveEvent* event) {

  Q_UNUSED(event);
}

void TilesetView::dropEvent(QDropEvent* event) {

  if (state != State::MOVING_PATTERNS) {
    return;
  }

  if (event->dropAction() == Qt::CopyAction) {
    event->acceptProposedAction();
  }
  else if (event->dropAction() == Qt::MoveAction) {
    event->acceptProposedAction();
  }
  end_state_moving_patterns();
}

/**
 * @brief Updates the position of the rectangle(s) the user is drawing or moving.
 *
 * In state DRAWING_RECTANGLE, if the specified area is the same as before,
 * nothing is done.
 *
 * @param start_point The starting point of drawing or moving.
 * @param current_point The current point of drawing or moving.
 */
void TilesetView::update_current_areas(
  const QPoint& start_point, const QPoint& current_point) {

  if (state == State::DRAWING_RECTANGLE) {

    QRect area = Rectangle::from_two_points(start_point, current_point);
    if (current_area_items.first()->rect().toRect() == area) {
      // No change.
      return;
    }
    current_area_items.first()->setRect(area);

    // Select items strictly in the rectangle.
    scene->clearSelection();
    QPainterPath path;
    path.addRect(QRect(area.topLeft() - QPoint(1, 1),
                       area.size() + QSize(2, 2)));
    scene->setSelectionArea(path, Qt::ContainsItemBoundingRect);

    // Re-select items that were already selected if Ctrl or Shift was pressed.
    for (QGraphicsItem* item : initially_selected_items) {
      item->setSelected(true);
    }
  }
}

/**
 * @brief Clears rectangle(s) the user is drawing or moving.
 */
void TilesetView::clear_current_areas() {

  for (QGraphicsRectItem* item : current_area_items) {
    scene->removeItem(item);
    delete item;
  }
  current_area_items.clear();
}

/**
 * @brief Returns all items that intersect the rectangles drawn by the user
 * except selected items.
 * @param ignore_selected @c true if the selection should be ignored.
 * @return The items that intersect the drawn rectangle.
 */
QList<QGraphicsItem*> TilesetView::get_items_intersecting_current_areas(
    bool ignore_selected) const {

  QList<QGraphicsItem*> items;

  for (QGraphicsRectItem* item : current_area_items) {
    QRect area = item->rect().toRect().adjusted(1, 1, -1, -1);
    items.append(scene->items(area, Qt::IntersectsItemBoundingRect));
    items.removeAll(item); // Ignore the drawn rectangle itself.
  }

  // Ignore selected items.
  if (ignore_selected) {
    const QList<QGraphicsItem*> selected_items = scene->selectedItems();
    for (QGraphicsItem* item : selected_items) {
      items.removeAll(item);
    }
  }

  return items;
}

/**
 * @brief Returns the bounding box corresponding to all selected tile patterns.
 * @return The bounding box of all the selected tile patterns.
 */
QRect TilesetView::get_selection_bounding_box() const {

  QRect bounding_box;
  const QList<int> selected_indexes = model->get_selected_indexes();
  for (int index : selected_indexes) {
    bounding_box = bounding_box.united(
      model->get_pattern_frames_bounding_box(index));
  }
  return bounding_box;
}

}
