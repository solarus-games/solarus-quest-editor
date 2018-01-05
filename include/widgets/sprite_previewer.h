/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_SPRITE_PREVIEWER_H
#define SOLARUSEDITOR_SPRITE_PREVIEWER_H

#include "ui_sprite_previewer.h"
#include <QWidget>
#include <QPointer>
#include <QTimer>
#include <QGraphicsPixmapItem>
#include <QGraphicsLineItem>
#include "sprite_model.h"

namespace SolarusEditor {

/**
 * @brief A widget to preview animation directions of sprites.
 */
class SpritePreviewer : public QWidget
{
  Q_OBJECT

public:

  SpritePreviewer(QWidget* parent = 0);

  void set_model(SpriteModel* model);

  void set_zoom(double zoom);
  void set_background_color(const QColor& color);
  void set_show_origin(bool show_origin);
  void set_origin_color(const QColor& color);

private slots:

  void update_selection();
  void update_frame_delay();

  void update_buttons();
  void update_frames();
  void update_frame();
  void update_origin();

  void timeout();

  void start();
  void stop();
  void first();
  void previous();
  void last();
  void next();

private slots:

  void update_zoom();

private:

  QMenu* create_zoom_menu();

  Ui::SpritePreviewer ui;       /**< The widgets. */
  QPointer<SpriteModel> model;  /**< The sprite model. */
  SpriteModel::Index index;     /**< The selected index. */
  QTimer timer;                 /**< The timer to animate the view. */
  QList<QPixmap> frames;        /**< Frames of the current direction. */
  int current_frame;            /**< Index of the current displayed frame. */
  QGraphicsPixmapItem* item;    /**< Pixmap item of the displayed frame. */
  QGraphicsLineItem* origin_h;  /**< Horizontal origin line. */
  QGraphicsLineItem* origin_v;  /**< Vertical origin line. */
  QMap<double, QAction*>
      zoom_actions;             /**< Action of each zoom value. */
  double zoom;                  /**< Zoom factor currently applied. */

};

}

#endif
