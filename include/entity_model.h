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
#ifndef SOLARUSEDITOR_ENTITY_MODEL_H
#define SOLARUSEDITOR_ENTITY_MODEL_H

#include <solarus/MapData.h>
#include <QPixmap>
#include <QPointer>

class MapModel;

using EntityIndex = Solarus::EntityIndex;

/**
 * @brief Data of a map entity.
 *
 * This class adds to a map entity all information
 * about how to represent and modify it in the editor.
 */
class EntityModel {

public:

  EntityModel(MapModel& map, const EntityIndex& index);
  virtual ~EntityModel();

  const MapModel& get_map() const;
  MapModel& get_map();

  virtual const QPixmap& get_image() const;

private:

  QPointer<MapModel> map;       /**< The map this entity belongs to. */
  EntityIndex index;            /**< Index of the entity on the map. */
  mutable QPixmap image;        /**< Image of the entity
                                 * to be displayed in the map view. */
};

#endif
