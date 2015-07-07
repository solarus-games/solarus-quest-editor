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
#ifndef SOLARUSEDITOR_SPRITE_VIEW_H
#define SOLARUSEDITOR_SPRITE_VIEW_H

#include <QGraphicsView>
#include <QPointer>
#include "sprite_model.h"

class SpriteScene;
class ViewSettings;
class QAction;

/**
 * @brief Graphical view of the sprite animation image, allowing to manage
 * directions.
 */
class SpriteView : public QGraphicsView {
  Q_OBJECT

public:

  SpriteView(QWidget* parent = nullptr);

  SpriteScene* get_scene();

  void set_model(SpriteModel* sprite);
  void set_view_settings(ViewSettings& view_settings);

signals:

  void delete_selected_direction_requested();
  void add_direction_requested(const QRect& frame);
  void change_selected_direction_position_requested(const QPoint& position);
  void duplicate_selected_direction_requested(const QPoint& position);

public slots:

  void update_zoom();
  void zoom_in();
  void zoom_out();
  void update_grid_visibility();

private slots:

  void duplicate_selected_direction_requested();

protected:

  void paintEvent(QPaintEvent* event) override;

  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;
  virtual void mouseMoveEvent(QMouseEvent* event) override;
  virtual void contextMenuEvent(QContextMenuEvent* event) override;

private:

  /**
   * @brief Possible operation the user is doing on this view.
   */
  enum class State {
      NORMAL,                   /**< Can click on directions. */
      DRAWING_RECTANGLE,        /**< Drawing a rectangle for a new direction. */
      MOVING_DIRECTION          /**< Moving an existing direcion to another
                                 * place in the PNG image. */
  };

  void show_context_menu(const QPoint& where);

  void start_state_normal();
  void start_state_drawing_rectangle(const QPoint& initial_point);
  void end_state_drawing_rectangle();
  void start_state_moving_direction(const QPoint& initial_point);
  void end_state_moving_direction();
  void set_current_area(const QRect& area);

  QPointer<SpriteModel> model;         /**< The sprite model. */
  SpriteScene* scene;                  /**< The scene viewed. */
  QAction* delete_direction_action;    /**< Action of deleting the selected
                                        * direction. */
  QAction* duplicate_direction_action; /**< Action of duplicate the selected
                                        * direction. */
  State state;                         /**< Current operation done by user. */
  QPoint dragging_start_point;         /**< In states DRAWING_RECTANGLE and
                                        * MOVING_DIRECTION: point where the
                                        * dragging started, in scene
                                        * coordinates.*/
  QPoint dragging_current_point;       /**< In states DRAWING_RECTANGLE and
                                        * MOVING_DIRECTION: point where the
                                        * dragging is currently, in scene
                                        * coordinates. */
  QGraphicsRectItem*
      current_area_item;               /**< In states DRAWING_RECTANGLE and
                                        * MOVING_DIRECTION: graphic item of the
                                        * rectangle the user is drawing. */
  QPointer<ViewSettings>
      view_settings;                   /**< How the view is displayed. */
  double zoom;                         /**< Zoom factor currently applied. */

};

#endif
