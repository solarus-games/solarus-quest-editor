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
#ifndef SOLARUSEDITOR_MAP_SCENE_H
#define SOLARUSEDITOR_MAP_SCENE_H

#include "map_model.h"
#include "view_settings.h"
#include <QGraphicsScene>

namespace SolarusEditor {

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
  const EntityModel* get_entity_from_item(const QGraphicsItem& item) const;
  EntityModel* get_entity_from_item(const QGraphicsItem& item);

  static QPoint get_margin_top_left();
  static QSize get_margin_size();

  void update_layer_visibility(int layer, const ViewSettings& view_settings);
  void update_traversables_visibility(const ViewSettings& view_settings);
  void update_obstacles_visibility(const ViewSettings& view_settings);
  void update_entity_type_visibility(EntityType type, const ViewSettings& view_settings);

  EntityIndexes get_selected_entities();
  void set_selected_entities(const EntityIndexes& indexes);
  void select_entity(const EntityIndex& index, bool selected);
  void select_all();
  void unselect_all();

  void redraw_entity(const EntityIndex& index);
  void redraw_entities(const EntityIndexes& indexes);

  int get_layer_in_rectangle(
      const QRect& rectangle
  ) const;

protected:

  void drawBackground(QPainter* painter, const QRectF& rect) override;

private slots:

  void size_changed(const QSize& size);
  void layer_range_changed(int min_layer, int max_layer);
  void entities_added(const EntityIndexes& indexes);
  void entities_about_to_be_removed(const EntityIndexes& indexes);
  void entity_layer_changed(const EntityIndex& index_before,
                            const EntityIndex& index_after);
  void entity_order_changed(const EntityIndex& index_before, int order_after);
  void entity_xy_changed(const EntityIndex& index, const QPoint& xy);
  void entity_size_changed(const EntityIndex& index, const QSize& size);

private:

  template<typename T>
  using ByLayer = QMap<int, T>;

  using EntityItems = QList<EntityItem*>;

  void build();
  void update_scene_size();
  void create_layer_parent_item(int layer);
  void create_entity_item(EntityModel& entity);
  EntityItem* get_entity_item(const EntityIndex& index);
  const EntityItems& get_entity_items(int layer);

  MapModel& map;                            /**< The map represented. */
  ByLayer<EntityItems> entity_items;        /**< Entities items on each layer,
                                             * ordered as in the map. */
  ByLayer<QGraphicsItem*>
      layer_parent_items;                   /**< Artificial parent item of everything on a layer. */

  QPointer<const ViewSettings>
      view_settings;                        /**< Last view settings applied. */
};

}

#endif
