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
#include "entities/crystal_block.h"
#include "map_model.h"

/**
 * @brief Constructor.
 * @param map The map containing the entity.
 * @param index Index of the entity in the map.
 */
CrystalBlock::CrystalBlock(MapModel& map, const EntityIndex& index) :
  EntityModel(map, index, EntityType::CRYSTAL_BLOCK) {

  set_resizable(true);
  set_base_size(QSize(16, 16));

  SubtypeList subtypes = {
    { "0", MapModel::tr("Initially lowered") },
    { "1", MapModel::tr("Initially raised") }
  };
  set_existing_subtypes(subtypes);
}

/**
 * @copydoc EntityModel::notify_field_changed
 */
void CrystalBlock::notify_field_changed(const QString& key, const QVariant& value) {

  EntityModel::notify_field_changed(key, value);

  if (key == "subtype") {
    update_subtype();
  }
}

/**
 * @brief Updates the representation of the crystal block.
 *
 * This function should be called when the subtype changes.
 */
void CrystalBlock::update_subtype() {

  DrawSpriteInfo info;
  info.sprite_id = "entities/crystal_block";
  info.animation = (get_subtype() == "0") ? "orange_lowered" : "blue_raised";
  info.frame = -1;
  info.tiled = true;
  set_draw_sprite_info(info);

}
