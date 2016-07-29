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
#ifndef SOLARUSEDITOR_SPRITE_VIEW_H
#define SOLARUSEDITOR_SPRITE_VIEW_H

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QPointer>
#include "sprite_model.h"

class QAction;

namespace SolarusEditor {

class SpriteScene;
class ViewSettings;

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
  void change_direction_num_frames_columns_requested(
    int num_frames, int num_columns);

public slots:

  void update_zoom();
  void zoom_in();
  void zoom_out();
  void update_grid_visibility();

private slots:

  void duplicate_selected_direction_requested();
  void change_num_frames_columns_requested();
  void change_num_columns_requested();
  void change_num_frames_requested();

protected:

  void paintEvent(QPaintEvent* event) override;

  virtual void focusOutEvent(QFocusEvent* event) override;
  virtual void keyPressEvent(QKeyEvent* event) override;
  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;
  virtual void mouseMoveEvent(QMouseEvent* event) override;
  virtual void contextMenuEvent(QContextMenuEvent* event) override;

private:

  /**
   * @brief Possible operation the user is doing on this view.
   */
  enum class State {
    NORMAL,                       /**< Can click on directions. */
    DRAWING_RECTANGLE,            /**< Drawing a rectangle for a new
                                   * direction. */
    MOVING_DIRECTION,             /**< Moving an existing direcion to another
                                   * place in the PNG image. */
    CHANGING_NUM_FRAMES_COLUMNS   /**< Changing the number of frames and
                                   * columns. */
  };

  /**
   * @brief Mode of changing the number of frames and columns operation.
   */
  enum class ChangingNumFramesColumnsMode {
    CHANGE_BOTH,          /**< Changing the number of frames and columns with
                             * the number of frames fixed to a multiple of the
                             * number of columns. */
    CHANGE_NUM_FRAMES,    /**< Changing the number of frames with the number
                             * of columns fixed. */
    CHANGE_NUM_COLUMNS    /**< Changing the number of columns with the number
                             * of frames fixed. */
  };

  /**
   * @brief Direction area item to display several frames.
   */
  class DirectionAreaItem : public QGraphicsItem {

  public:

    DirectionAreaItem();

    void set_frame_size(const QSize& frame_size);
    void set_num_frames(int num_frames);
    void set_num_columns(int num_columns);

    QRect get_direction_all_frames_rect();

    virtual QRectF boundingRect() const override;

  protected:

    virtual void paint(
      QPainter* painter, const QStyleOptionGraphicsItem* option,
      QWidget* widget = nullptr) override;

  private:

    void update_bouding_rect();

    QSize frame_size;     /**< The size of a frame. */
    QRect bounding_rect;  /**< The current bounding rect. */
    int num_frames;       /**< The number of frames. */
    int num_columns;      /**< The number of columns. */

  };

  void show_context_menu(const QPoint& where);

  void change_num_frames_columns(const ChangingNumFramesColumnsMode& mode);

  void start_state_normal();
  void start_state_drawing_rectangle(const QPoint& initial_point);
  void end_state_drawing_rectangle();
  void start_state_moving_direction(const QPoint& initial_point);
  void end_state_moving_direction();
  void start_state_changing_num_frames_columns(
    const ChangingNumFramesColumnsMode& mode);
  void update_state_changing_num_frames_columns(const QPoint& current_point);
  void end_state_changing_num_frames_columns();
  void cancel_state_changing_num_frames_columns();

  QPointer<SpriteModel> model;         /**< The sprite model. */
  SpriteScene* scene;                  /**< The scene viewed. */
  QAction* delete_direction_action;    /**< Action of deleting the selected
                                        * direction. */
  QAction* duplicate_direction_action; /**< Action of duplicate the selected
                                        * direction. */
  QAction*
    change_num_frames_columns_action;  /**< Action of change the number of
                                        * frames and columns of the selected
                                        * direction. */
  QAction* change_num_frames_action;   /**< Action of change the number of
                                        * frames of the selected direction. */
  QAction* change_num_columns_action;  /**< Action of change the number of
                                        * columns of the selected direction. */
  State state;                         /**< Current operation done by user. */
  ChangingNumFramesColumnsMode
      changing_mode;                   /**< Current mode for changing the number
                                        * of frames and columns operation. */
  QPoint dragging_start_point;         /**< In states DRAWING_RECTANGLE and
                                        * MOVING_DIRECTION: point where the
                                        * dragging started, in scene
                                        * coordinates.*/
  QPoint dragging_current_point;       /**< In states DRAWING_RECTANGLE,
                                        * MOVING_DIRECTION and
                                        * CHANGING_NUM_FRAMES_COLUMNS: point
                                        * where the dragging is currently,
                                        * in scene coordinates. */
  DirectionAreaItem current_area_item; /**< In states DRAWING_RECTANGLE,
                                        * MOVING_DIRECTION and
                                        * CHANGING_NUM_FRAMES_COLUMNS:
                                        * graphic item of the area drawing
                                        * by the user. */
  QPointer<ViewSettings>
      view_settings;                   /**< How the view is displayed. */
  double zoom;                         /**< Zoom factor currently applied. */

};

}

#endif
