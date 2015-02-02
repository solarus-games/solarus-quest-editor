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
#ifndef SOLARUSEDITOR_MAP_VIEW_H
#define SOLARUSEDITOR_MAP_VIEW_H

#include <QGraphicsView>
#include <QPointer>

class MapModel;
class MapScene;

/**
 * @brief Graphical view of the map image, allowing to manage entities.
 */
class MapView : public QGraphicsView {
  Q_OBJECT

public:

  MapView(QWidget* parent = nullptr);

  void set_model(MapModel* map);
  double get_zoom() const;

signals:

  void zoom_changed(double zoom);

public slots:

  void set_zoom(double zoom);
  void zoom_in();
  void zoom_out();

private:

  QPointer<MapModel> model;        /**< The map model. */
  MapScene* scene;                 /**< The scene viewed. */

  double zoom;                     /**< Scale factor of the view. */

};

#endif
