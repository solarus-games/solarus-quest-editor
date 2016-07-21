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
#include "entities/destination.h"
#include "map_model.h"

namespace SolarusEditor {

/**
 * @brief Constructor.
 * @param map The map containing the entity.
 * @param index Index of the entity in the map.
 */
Destination::Destination(MapModel& map, const EntityIndex& index) :
  EntityModel(map, index, EntityType::DESTINATION),
  update_teletransporters(true) {

  set_origin(QPoint(8, 13));

  set_num_directions(4);
  set_no_direction_allowed(true);
  set_no_direction_text(MapModel::tr("Keep the same direction"));

  // Draw an image that depends on a direction.
  DrawImageInfo info;
  QString file_name(":/images/entity_destination_all.png");
  info.scale = 2.0;
  info.image_no_direction.file_name = file_name;
  info.image_no_direction.src_rect = QRect(0, 0, 32, 32);
  for (int direction = 0; direction < 4; ++direction) {
    SubImage sub_image;
    sub_image.file_name = file_name;
    sub_image.src_rect = QRect((direction + 1) * 32, 0, 32, 32);
    info.images_by_direction.append(sub_image);
  }
  set_draw_image_info(info);
}

/**
 * @copydoc EntityModel::set_initial_values
 */
void Destination::set_initial_values() {

  EntityModel::set_initial_values();

  // Set a default name to avoid surprises with destinations that save
  // the starting location.
  // (Its uniqueness will be ensured later by appending a suffix.)
  set_name("destination");
}

/**
 * @brief Returns whether existing teletransporters should be updated when
 * the name of this destination changes.
 * @return @c true if teletransporters should be updated.
 */
bool Destination::get_update_teletransporters() const {
  return update_teletransporters;
}

/**
 * @brief Sets whether existing teletransporters should be updated when
 * the name of this destination changes.
 * @param update_teletransporters @c true if teletransporters should be updated.
 */
void Destination::set_update_teletransporters(bool update_teletransporters) {
  this->update_teletransporters = update_teletransporters;
}

}
