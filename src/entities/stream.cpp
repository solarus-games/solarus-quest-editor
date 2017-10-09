/*
 * Copyright (C) 2014-2017 Christopho, Solarus - http://www.solarus-games.org
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
#include "entities/stream.h"

namespace SolarusEditor {

/**
 * @brief Constructor.
 * @param map The map containing the entity.
 * @param index Index of the entity in the map.
 */
Stream::Stream(MapModel& map, const EntityIndex& index) :
  EntityModel(map, index, EntityType::STREAM) {

  set_origin(QPoint(8, 13));
  set_num_directions(8);

  // When no sprite is set, draw an image that depends on a direction.
  DrawImageInfo info;
  QString file_name(":/images/entity_stream_all.png");
  for (int direction = 0; direction < 8; ++direction) {
    SubImage sub_image;
    sub_image.file_name = file_name;
    sub_image.src_rect = QRect(direction * 16, 0, 16, 16);
    info.images_by_direction.append(sub_image);
  }
  set_draw_image_info(info);
}

}
