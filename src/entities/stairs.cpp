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
#include "entities/stairs.h"
#include "map_model.h"

namespace SolarusEditor {

/**
 * @brief Constructor.
 * @param map The map containing the entity.
 * @param index Index of the entity in the map.
 */
Stairs::Stairs(MapModel& map, const EntityIndex& index) :
  EntityModel(map, index, EntityType::STAIRS) {

  set_num_directions(4);

  SubtypeList subtypes = {
    { "0", MapModel::tr("Spiral staircase (going upstairs)") },
    { "1", MapModel::tr("Spiral staircase (going downstairs)") },
    { "2", MapModel::tr("Straight staircase (going upstairs)") },
    { "3", MapModel::tr("Straight staircase (going downstairs)") },
    { "4", MapModel::tr("Platform stairs (same map)") }
  };
  set_existing_subtypes(subtypes);
  set_traversable(false);
}

/**
 * @copydoc EntityModel::notify_field_changed
 */
void Stairs::notify_field_changed(const QString& key, const QVariant& value) {

  EntityModel::notify_field_changed(key, value);

  if (key == "subtype") {
    update_drawing_info();
  }
}

/**
 * @brief Updates the representation of the stairs.
 *
 * This function should be called when the subtype or direction changes.
 */
void Stairs::update_drawing_info() {

  QString subtype = get_subtype();
  bool ok = false;
  int subtype_index = subtype.toInt(&ok);
  Q_ASSERT(ok);
  int x = subtype_index * 32;

  DrawImageInfo info;
  for (int i = 0; i < 4; ++i) {
    int y = i * 32;
    SubImage sub_image;
    sub_image.file_name = ":/images/entity_stairs_all.png";
    sub_image.src_rect = QRect(x, y, 32, 32);
    info.images_by_direction << sub_image;
    info.scale = 2.0;
  }
  set_draw_image_info(info);
}

}
