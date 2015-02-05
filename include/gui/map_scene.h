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
#ifndef SOLARUSEDITOR_MAP_SCENE_H
#define SOLARUSEDITOR_MAP_SCENE_H

#include "layer_traits.h"
#include "map_model.h"
#include <QGraphicsScene>

class EntityItem;
class Quest;
class ViewSettings;

/**
 * @brief The scene containing all entities in the map main view.
 *
 * In scene coordinates, map entity coordinates are multiplied by 2.
 * This allows us to show entity icons at a higher resolution than tiles:
 * for example, we show a 32x32 icon on a 16x16 tile
 * In scene coordinates, a tile with coordinates 16x16 in the data file becomes 32x32.
 * In other words, scene coordinates match the view when the user actually
 * sets a zoom of 200%.
 * This is nice for the user, but in the code all coordinates from the map
 * data file have to be scaled.
 */
class MapScene : public QGraphicsScene {
  Q_OBJECT

public:

  MapScene(MapModel& model, QObject* parent);

  const MapModel& get_model() const;
  const Quest& get_quest() const;

  void update_layer_visibility(Layer layer, const ViewSettings& view_settings);
  void update_entity_type_visibility(EntityType type, const ViewSettings& view_settings);

  static int quest_to_scene(int coordinate);
  static int scene_to_quest(int coordinate);
  static double quest_to_scene(double coordinate);
  static double scene_to_quest(double coordinate);
  static QPoint quest_to_scene(const QPoint& point);
  static QPoint scene_to_quest(const QPoint& point);
  static QRect quest_to_scene(const QRect& rectangle);
  static QRect scene_to_quest(const QRect& rectangle);
  static QSize quest_to_scene(const QSize& size);
  static QSize scene_to_quest(const QSize& size);

  static constexpr int quest_to_scene_factor = 2;

private:

  using EntityList = QList<EntityItem*>;

  void build();
  void create_entity_item(const EntityIndex& index);
  const EntityList& get_entities(Layer layer);

  MapModel& model;            /**< The tileset represented. */
  std::array<EntityList, Layer::LAYER_NB>
      entity_items;           /**< Entities item in the scene on each layer,
                               * ordered as in the model. */

};

#endif
