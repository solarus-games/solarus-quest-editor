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
#ifndef SOLARUSEDITOR_VIEW_SETTINGS_H
#define SOLARUSEDITOR_VIEW_SETTINGS_H

#include "entities/entity_traits.h"
#include "grid_style.h"
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

  GridStyle get_grid_style() const;
  void set_grid_style(GridStyle style);

  QColor get_grid_color() const;
  void set_grid_color(const QColor& color);

  void get_layer_range(int& min_layer, int& max_layer) const;
  void set_layer_range(int min_layer, int max_layer);
  void set_layer_visible(int layer, bool visible);
  bool is_layer_visible(int layer) const;
  void show_all_layers();
  void hide_all_layers();

  bool is_entity_type_visible(EntityType entity_type) const;
  void set_entity_type_visible(EntityType entity_type, bool visible);
  void show_all_entity_types();
  void hide_all_entity_types();

signals:

  void zoom_changed(double zoom);
  void grid_visibility_changed(bool grid_visible);
  void grid_size_changed(const QSize& size);
  void grid_style_changed(GridStyle style);
  void grid_color_changed(const QColor& color);
  void layer_range_changed(int min_layer, int max_layer);
  void layer_visibility_changed(int layer, bool visible);
  void entity_type_visibility_changed(EntityType entity_type, bool visible);

private:

  double zoom;                              /**< If supported, the current zoom factor. */
  bool grid_visible;                        /**< If supported, whether the grid is currently shown.*/
  QSize grid_size;                          /**< If supported, the current grid size. */
  GridStyle grid_style;                     /**< If supported, the current grid style. */
  QColor grid_color;                        /**< If supported, the current grid color. */
  int min_layer;                            /**< Lowest layer in the editor
                                             * (0 if showing/hiding layers is not supported). */
  int max_layer;                            /**< Highest layer in the editor
                                             * (-1 if showing/hiding layers is not supported). */
  std::set<int> visible_layers;             /**< Layers currently shown, if supported. */
  std::set<EntityType>
      visible_entity_types;                 /**< Types of entities currently shown, if supported. */

};

#endif
