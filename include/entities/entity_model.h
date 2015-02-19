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

#include "entity_traits.h"
#include "layer_traits.h"
#include <solarus/MapData.h>
#include <QPixmap>
#include <QPointer>
#include <memory>

class MapModel;
class Quest;
class QuestResources;
class TilesetModel;
class QPainter;

using EntityIndex = Solarus::EntityIndex;

/**
 * @brief Model of a map entity.
 *
 * This class wraps an enttiy from the Solarus library and
 * adds all useful information about how to represent and modify it in the
 * editor.
 */
class EntityModel {

public:

  EntityModel(MapModel& map, const Solarus::EntityData& entity);  // TODO protected
  virtual ~EntityModel();
  static std::unique_ptr<EntityModel> create(
      MapModel& map, const Solarus::EntityData& entity_data);

  const MapModel& get_map() const;
  MapModel& get_map();
  const TilesetModel* get_tileset() const;
  const Quest& get_quest() const;
  const QuestResources& get_resources() const;

  EntityType get_type() const;
  QString get_type_name() const;

  Layer get_layer() const;
  void set_layer(Layer layer);
  QPoint get_xy() const;
  void set_xy(const QPoint& xy);
  QPoint get_top_left() const;
  void set_top_left(const QPoint& top_left);
  QPoint get_origin() const;
  void set_origin(const QPoint& origin);
  int get_width() const;
  void set_width(int width);
  int get_height() const;
  void set_height(int height);
  QSize get_size() const;
  void set_size(const QSize& size);
  QRect get_bounding_box() const;
  QVariant get_field(const QString& key) const;

  virtual void draw(QPainter& painter) const;

private:

  bool draw_as_sprite(QPainter& painter) const;
  void draw_as_icon(QPainter& painter) const;

  QPointer<MapModel> map;       /**< The map this entity belongs to.
                                 * (could be a reference but we want operator=) */
  Solarus::EntityData entity;   /**< The entity data wrapped. */
  QPoint origin;                /**< Origin point of the entity. */
  QSize size;                   /**< Size of the entity for the editor. */
  mutable QPixmap icon;         /**< Icon of the entity
                                 * to be displayed by the default. */
  mutable QPixmap sprite;       /**< Sprite image of the entity to be displayed. */
};

#endif
