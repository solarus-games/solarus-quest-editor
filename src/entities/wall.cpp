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
#include "entities/wall.h"

namespace SolarusEditor {

/**
 * @brief Constructor.
 * @param map The map containing the entity.
 * @param index Index of the entity in the map.
 */
Wall::Wall(MapModel& map, const EntityIndex& index) :
  EntityModel(map, index, EntityType::WALL) {

  set_resizable(true);
  set_base_size(QSize(8, 8));

  DrawShapeInfo info;
  info.enabled = true;
  info.between_border_color = QColor(240, 142, 142);
  info.pixmap = QPixmap(":/images/entity_wall_resizable.png");
  info.tiled_pixmap = true;
  set_draw_shape_info(info);
}

/**
 * @copydoc EntityModel::set_initial_values
 */
void Wall::set_initial_values() {

  EntityModel::set_initial_values();

  // Default values in the data file format are poor: the wall is traversable
  // by everything.
  set_field("stops_hero", true);
  set_field("stops_npcs", true);
  set_field("stops_enemies", true);
  set_field("stops_blocks", true);
  set_field("stops_projectiles", true);
}

/**
 * @copydoc EntityModel::notify_field_changed
 */
void Wall::notify_field_changed(const QString& key, const QVariant& value) {

  EntityModel::notify_field_changed(key, value);

  if (key == "stops_hero") {
    bool stops_hero = value.toBool();
    set_traversable(!stops_hero);
  }
}

}
