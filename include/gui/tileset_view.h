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
#ifndef SOLARUSEDITOR_TILESET_VIEW_H
#define SOLARUSEDITOR_TILESET_VIEW_H

#include "ground_traits.h"
#include "layer_traits.h"
#include "pattern_animation.h"
#include "pattern_separation.h"
#include <QGraphicsView>
#include <QPointer>

class TilesetModel;
class TilesetScene;
class ViewSettings;
class QAction;

/**
 * @brief Graphical view of the tileset image, allowing to manage tile patterns.
 */
class TilesetView : public QGraphicsView {
  Q_OBJECT

public:

  TilesetView(QWidget* parent = nullptr);

  void set_model(TilesetModel* tileset);
  void set_view_settings(ViewSettings& view_settings);
  bool is_read_only() const;

signals:

  void create_pattern_requested(
      const QString& pattern_id, const QRect& frame, Ground ground);
  void delete_selected_patterns_requested();
  void change_selected_pattern_id_requested();
  void change_selected_pattern_position_requested(const QPoint& position);
  void change_selected_patterns_ground_requested(Ground ground);
  void change_selected_patterns_default_layer_requested(Layer layer);
  void change_selected_patterns_animation_requested(PatternAnimation animation);
  void change_selected_patterns_separation_requested(PatternSeparation separation);

public slots:

  void set_read_only(bool read_only);
  void update_zoom();
  void zoom_in();
  void zoom_out();
  void update_grid_visibility();

protected:

  void drawForeground(QPainter* painter, const QRectF& rectangle) override;

  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;

private:

  /**
   * @brief Possible operation the user is doing on this view.
   */
  enum class State {
      NORMAL,                   /**< Can click on patterns. */
      DRAWING_RECTANGLE,        /**< Drawing a rectangle for a selection or
                                 * a new pattern. */
      MOVING_PATTERN            /**< Moving an existing pattern to another
                                 * place in the PNG image. */
  };

  void show_context_menu(const QPoint& where);
  void build_context_menu_ground(QMenu& menu, const QList<int>& indexes);
  void build_context_menu_layer(QMenu& menu, const QList<int>& indexes);
  void build_context_menu_animation(QMenu& menu, const QList<int>& indexes);

  void start_state_normal();
  void start_state_drawing_rectangle(const QPoint& initial_point);
  void end_state_drawing_rectangle();
  void start_state_moving_pattern(const QPoint& initial_point);
  void end_state_moving_pattern();
  void set_current_area(const QRect& area);
  QList<QGraphicsItem*> get_items_in_current_area() const;
  QList<QGraphicsItem*> get_items_intersecting_current_area() const;

  QPointer<TilesetModel> model;        /**< The tileset model. */
  TilesetScene* scene;                 /**< The scene viewed. */
  QAction* change_pattern_id_action;   /**< Action of changing a pattern id. */
  QAction* delete_patterns_action;     /**< Action of deleting the selected
                                        * patterns. */
  int last_integer_pattern_id;         /**< Last auto-generated pattern id. */
  State state;                         /**< Current operation done by user. */
  QPoint dragging_start_point;         /**< In states DRAWING_NEW_PATTERN and
                                        * MOVING_PATTERN: point where the
                                        * dragging started, in scene coordinates.*/
  QPoint dragging_current_point;       /**< In states DRAWING_NEW_PATTERN and
                                        * MOVING_PATTERN: point where the
                                        * dragging is currently, in scene coordinates. */
  QGraphicsRectItem*
      current_area_item;               /**< In states DRAWING_NEW_PATTERN and
                                        * MOVING_PATTERN: graphic item of the
                                        * rectangle the user is drawing. */
  QPointer<ViewSettings>
      view_settings;                   /**< How the view is displayed. */
  double zoom;                         /**< Zoom factor currently applied. */

  bool read_only;                      /**< Whether the view forbids editing the tileset. */

};

#endif
