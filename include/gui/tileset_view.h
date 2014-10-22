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
#ifndef SOLARUSEDITOR_TILESET_VIEW_H
#define SOLARUSEDITOR_TILESET_VIEW_H

#include <QGraphicsView>
#include <QPointer>

class TilesetModel;
class QAction;

/**
 * @brief Graphical view of the tileset image, allowing to manage tile patterns.
 */
class TilesetView : public QGraphicsView {
  Q_OBJECT

public:

  TilesetView(QWidget* parent = nullptr);

  void set_model(TilesetModel& tileset);
  double get_zoom() const;

signals:

  void change_selected_pattern_id_requested();
  void delete_selected_patterns_requested();

public slots:

  void set_zoom(double zoom);
  void zoom_in();
  void zoom_out();

protected:

  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;
  virtual void mouseMoveEvent(QMouseEvent* event) override;
  virtual void wheelEvent(QWheelEvent* event) override;

private:

  void show_context_menu(const QPoint& where);

  QPointer<TilesetModel> model;        /**< The tileset viewed. */
  QAction* change_pattern_id_action;   /**< Action of changing a pattern id. */
  QAction* delete_patterns_action;     /**< Action of deleting the selected
                                        * patterns. */

  // TODO move the panning code to a reusable class,
  // because other views also need it. Use an event filter?
  QPoint pan_initial_point;            /**< Point initially clicked when panning
                                        * the view, in scrolling-independent
                                        * coordinates. */

  double zoom;                         /**< Scale factor of the view. */
};

#endif
