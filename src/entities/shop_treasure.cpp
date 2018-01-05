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
#include "entities/shop_treasure.h"

namespace SolarusEditor {

/**
 * @brief Constructor.
 * @param map The map containing the entity.
 * @param index Index of the entity in the map.
 */
ShopTreasure::ShopTreasure(MapModel& map, const EntityIndex& index) :
  EntityModel(map, index, EntityType::SHOP_TREASURE) {

  set_size(QSize(32, 32));

  DrawShapeInfo info;
  info.enabled = true;
  info.background_color = QColor(224, 108, 72);
  info.between_border_color = QColor(184, 96, 96);
  info.pixmap = QPixmap(":/images/entity_shop_treasure.png");
  set_draw_shape_info(info);
  set_traversable(false);
}

/**
 * @copydoc EntityModel::set_initial_values
 */
void ShopTreasure::set_initial_values() {

  EntityModel::set_initial_values();

  // The default price in the data file format is poor (0).
  set_field("price", 10);
}

}
