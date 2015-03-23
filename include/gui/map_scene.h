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
#ifndef SOLARUSEDITOR_MAP_SCENE_H
#define SOLARUSEDITOR_MAP_SCENE_H

#include "layer_traits.h"
#include "map_model.h"
#include <QGraphicsScene>

class EntityItem;
class Quest;
class ViewSettings;

/**
 * @brief The scene containing all entities in the map main view.
 */
class MapScene : public QGraphicsScene {
  Q_OBJECT

public:

  MapScene(MapModel& model, QObject* parent);

  const MapModel& get_model() const;
  const Quest& get_quest() const;
  EntityModel* get_entity_from_item(const QGraphicsItem& item);

  static QPoint get_margin_top_left();
  static QSize get_margin_size();

  void update_layer_visibility(Layer layer, const ViewSettings& view_settings);
  void update_entity_type_visibility(EntityType type, const ViewSettings& view_settings);

  QList<EntityIndex> get_selected_entities();
  void set_selected_entities(const QList<EntityIndex>& indexes);

protected:

  void drawBackground(QPainter* painter, const QRectF& rect) override;

private slots:

  void entities_added(const QList<EntityIndex>& indexes);
  void entities_about_to_be_removed(const QList<EntityIndex>& indexes);
  void entity_xy_changed(const EntityIndex& index, const QPoint& xy);
  void entity_size_changed(const EntityIndex& index, const QSize& size);

private:

  using EntityItems = QList<EntityItem*>;

  void build();
  void create_entity_item(EntityModel& entity);
  EntityItem* get_entity_item(const EntityIndex& index);
  const EntityItems& get_entity_items(Layer layer);

  MapModel& map;              /**< The map represented. */
  std::array<EntityItems, Layer::LAYER_NB>
      entity_items;           /**< Entities items on each layer,
                               * ordered as in the map. */

};

#endif
