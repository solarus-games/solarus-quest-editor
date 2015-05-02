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
#ifndef SOLARUSEDITOR_VIEW_SETTINGS_H
#define SOLARUSEDITOR_VIEW_SETTINGS_H

#include "entities/entity_traits.h"
#include "layer_traits.h"
#include <QObject>
#include <set>

/**
 * @brief Stores the view settings of an editor.
 *
 * View settings include the current zoom, whether the grid is shown
 * and whether map entities are shown.
 */
class ViewSettings : public QObject {
  Q_OBJECT

public:

  ViewSettings(QObject* parent = nullptr);

  double get_zoom() const;
  void set_zoom(double zoom);

  bool is_grid_visible() const;
  void set_grid_visible(bool grid_visible);

  QSize get_grid_size() const;
  void set_grid_size(const QSize& size);

  void set_layer_visible(Layer layer, bool visible);
  bool is_layer_visible(Layer layer) const;

  bool is_entity_type_visible(EntityType entity_type) const;
  void set_entity_type_visible(EntityType entity_type, bool visible);
  void show_all_entity_types();
  void hide_all_entity_types();

signals:

  void zoom_changed(double zoom);
  void grid_visibility_changed(bool grid_visible);
  void grid_size_changed(const QSize& size);
  void layer_visibility_changed(Layer layer, bool visible);
  void entity_type_visibility_changed(EntityType entity_type, bool visible);

private:

  double zoom;                              /**< If supported, the current zoom factor. */
  bool grid_visible;                        /**< If supported, whether the grid is currently shown.*/
  QSize grid_size;                          /**< If supported, the current grid size. */
  bool layer_visibility_supported;          /**< Whether the editor supports showing/hiding layers. */
  std::set<Layer> visible_layers;           /**< Layers currently shown, if supported. */
  bool entity_type_visibility_supported;    /**< Whether the editor supports showing/hiding entity types. */
  std::set<EntityType>
      visible_entity_types;                 /**< Types of entities currently shown, if supported. */

};

#endif
