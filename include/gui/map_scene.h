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

#include <layer_traits.h>
#include <map_model.h>
#include <QGraphicsScene>

class EntityItem;
class Quest;
class MapModel;

/**
 * @brief The scene containing all entities in the map main view.
 */
class MapScene : public QGraphicsScene {
  Q_OBJECT

public:

  MapScene(MapModel& model, QObject* parent);

  const MapModel& get_model() const;
  const Quest& get_quest() const;

private:

  void build();
  void create_entity_item(const EntityIndex& index);

  MapModel& model;            /**< The tileset represented. */
  std::array<QList<EntityItem*>, Layer::LAYER_NB>
      entity_items;           /**< Entities item in the scene on each layer,
                               * ordered as in the model. */

};

#endif
