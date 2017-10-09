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
#ifndef SOLARUSEDITOR_ENTITY_ITEM_H
#define SOLARUSEDITOR_ENTITY_ITEM_H

#include "entities/entity_traits.h"
#include <QGraphicsItem>

namespace SolarusEditor {

class EntityModel;
class ViewSettings;

/**
 * @brief Graphic item representing a map entity.
 */
class EntityItem : public QGraphicsItem {

public:

  // Enable the use of qgraphicsitem_cast with this item.
  enum {
    Type = UserType + 2
  };

  int type() const override {
    return Type;
  }

  EntityItem(EntityModel& entity, QGraphicsItem* parent = nullptr);

  EntityModel& get_entity() const;
  EntityIndex get_index() const;
  EntityType get_entity_type() const;
  QRectF boundingRect() const override;

  void update_visibility(const ViewSettings& view_settings);
  void update_xy();
  void update_size();

protected:

  void paint(QPainter* painter,
             const QStyleOptionGraphicsItem* option,
             QWidget* widget = nullptr) override;

private:

  EntityModel& entity;      /**< The entity represented. */
  QSize size;               /**< Current size of the item.
                             * TODO for some entities like NPC, it could be larger
                             * than the entity's bounding box because of sprites. */

};

}

#endif
