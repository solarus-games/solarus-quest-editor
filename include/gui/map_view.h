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

#include "entities/entity_traits.h"
#include "layer_traits.h"
#include <QGraphicsView>
#include <QPointer>

class MapModel;
class MapScene;
class ViewSettings;

/**
 * @brief Graphical view of the map image, allowing to manage entities.
 */
class MapView : public QGraphicsView {
  Q_OBJECT

public:

  MapView(QWidget* parent = nullptr);

  void set_model(MapModel* map);
  void set_view_settings(ViewSettings& view_settings);

public slots:

  void update_zoom();
  void zoom_in();
  void zoom_out();
  void update_grid_visibility();
  void update_layer_visibility(Layer layer);
  void update_entity_type_visibility(EntityType type);

protected:

  void drawForeground(QPainter* painter, const QRectF& rectangle);

private:

  QPointer<MapModel> model;        /**< The map model. */
  MapScene* scene;                 /**< The scene viewed. */
  QPointer<ViewSettings>
      view_settings;               /**< What is displayed in the view. */
  double zoom;                     /**< Zoom factor currently applied. */

};

#endif
