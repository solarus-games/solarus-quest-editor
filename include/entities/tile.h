/*
 * Copyright (C) 2014-2016 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_TILE_H
#define SOLARUSEDITOR_TILE_H

#include "entity_model.h"

namespace SolarusEditor {

/**
 * @brief An editable tile.
 */
class Tile : public EntityModel {

public:

  Tile(MapModel& map, const EntityIndex& index);
  static EntityModelPtr create_from_dynamic_tile(MapModel& map, const EntityIndex& dynamic_tile_index);

  QString get_pattern_id() const;
  void set_pattern_id(const QString& pattern_id);

  void draw(QPainter& painter) const override;
  void notify_tileset_changed(const QString& tileset_id) override;

protected:

  Tile(MapModel& map, const EntityIndex& index, EntityType type);

  void notify_field_changed(const QString& key, const QVariant& value) override;

private:

  void update_pattern();
  ResizeMode get_pattern_resize_mode() const;

  mutable QPixmap pattern_image;     /**< Cached image of the tile pattern. */

};

}

#endif
