/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
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
#include "view_settings.h"

namespace SolarusEditor {

/**
 * @brief Creates default view settings.
 * @param parent The parent object or nullptr.
 */
ViewSettings::ViewSettings(QObject* parent) :
  QObject(parent),
  zoom(1.0),
  grid_visible(false),
  grid_size(16, 16),
  grid_style(GridStyle::DASHED),
  grid_color(Qt::black),
  min_layer(0),
  max_layer(-1),
  locked_layers(),
  visible_layers(),
  traversables_visible(true),
  obstacles_visible(true),
  visible_entity_types() {

  // Default settings.
  const QList<EntityType>& entity_types = EntityTraits::get_values();
  for (EntityType entity_type : entity_types) {
    visible_entity_types.insert(entity_type);
  }
}

/**
 * @brief Returns the current zoom factor.
 * @return The zoom factor.
 */
double ViewSettings::get_zoom() const {
  return zoom;
}

/**
 * @brief Sets the zoom factor.
 *
 * Emits zoom_changed() if there is a change.
 *
 * @param zoom The zoom factor to set.
 */
void ViewSettings::set_zoom(double zoom) {

  if (zoom == this->zoom) {
    return;
  }

  this->zoom = zoom;
  emit zoom_changed(zoom);
}

/**
 * @brief Returns the current grid visibility setting.
 * @return @c true if the grid is visible, @c false if it is hidden
 * or not supported.
 */
bool ViewSettings::is_grid_visible() const {

  return grid_visible;
}

/**
 * @brief Shows or hides the grid.
 *
 * Emits grid_visibility_changed() if there is a change.
 *
 * @param grid_visible @c true to show the grid, @c false to hide it.
 */
void ViewSettings::set_grid_visible(bool grid_visible) {

  if (grid_visible == this->grid_visible) {
    return;
  }

  this->grid_visible = grid_visible;
  emit grid_visibility_changed(grid_visible);
}

/**
 * @brief Returns the current grid size setting.
 * @return the current grid size.
 */
QSize ViewSettings::get_grid_size() const {

  return grid_size;
}

/**
 * @brief Changes the current grid size.
 *
 * Emits grid_size_changed() if there is a change and if the grid is visible.
 *
 * @param size the new grid size.
 */
void ViewSettings::set_grid_size(const QSize& size) {

  if (size == grid_size) {
    return;
  }

  grid_size = size;

  if (grid_visible) {
    emit grid_size_changed(grid_size);
  }
}

/**
 * @brief Returns the current grid style setting.
 * @return the current grid style.
 */
GridStyle ViewSettings::get_grid_style() const {

  return grid_style;
}

/**
 * @brief Changes the current grid style.
 *
 * Emits grid_style_changed() if there is a change and if the grid is visible.
 *
 * @param style the new grid style.
 */
void ViewSettings::set_grid_style(GridStyle style) {

  if (style == grid_style) {
    return;
  }

  grid_style = style;

  if (grid_visible) {
    emit grid_style_changed(grid_style);
  }
}

/**
 * @brief Returns the current grid color setting.
 * @return the current grid color.
 */
QColor ViewSettings::get_grid_color() const {

  return grid_color;
}

/**
 * @brief Changes the current grid color.
 *
 * Emits grid_color_changed() if there is a change and if the grid is visible.
 *
 * @param color the new grid color.
 */
void ViewSettings::set_grid_color(const QColor& color) {

  if (color == grid_color || !color.isValid()) {
    return;
  }

  grid_color = color;

  if (grid_visible) {
    emit grid_color_changed(grid_color);
  }
}

/**
 * @brief Returns the range of layers supported for visibility.
 *
 * Returns an empty range if layer visibility is not supported.
 *
 * @param[ou] min_layer The lowest layer.
 * @param[ou] max_layer The highest layer.
 */
void ViewSettings::get_layer_range(int& min_layer, int& max_layer) const {

  min_layer = this->min_layer;
  max_layer = this->max_layer;
}

/**
 * @brief Sets the range of layers supported for visibility.
 *
 * Emits layer_range_changed().
 * Calling this function resets all layers to visible.
 *
 * @param min_layer The lowest layer.
 * @param max_layer The lowest layer.
 */
void ViewSettings::set_layer_range(int min_layer, int max_layer) {

  if (min_layer == this->min_layer && max_layer == this->max_layer) {
    return;
  }

  if (max_layer < min_layer) {
    // Empty range: layer visibility is not supported.
    min_layer = 0;
    max_layer = -1;
  }

  this->min_layer = min_layer;
  this->max_layer = max_layer;

  emit layer_range_changed(min_layer, max_layer);

  locked_layers.clear();
  visible_layers.clear();
  show_all_layers();
}

/**
 * @brief Returns whether a layer is currently locked.
 * @param layer The layer to test.
 * @return @c true if this layer is locked, @c false if it is unlocked.
 */
bool ViewSettings::is_layer_locked(int layer) const {

  return locked_layers.find(layer) != locked_layers.end();
}

/**
 * @brief Locks or unlocks a layer.
 *
 * Emits layer_locking_changed() if there is a change.
 *
 * @param layer The layer to change.
 * @param locked @c true to lock the layer, @c false to unlock it.
 */
void ViewSettings::set_layer_locked(int layer, bool locked) {

  Q_ASSERT(layer >= min_layer && layer <= max_layer);

  if (locked == is_layer_locked(layer)) {
    return;
  }

  if (locked) {
    locked_layers.insert(layer);
  }
  else {
    locked_layers.erase(layer);
  }
  emit layer_locking_changed(layer, locked);
}

/**
 * @brief Returns whether a layer is currently visible.
 * @param layer The layer to test.
 * @return @c true if this layer is visible, @c false if it is hidden.
 */
bool ViewSettings::is_layer_visible(int layer) const {

  return visible_layers.find(layer) != visible_layers.end();
}

/**
 * @brief Shows or hides a layer.
 *
 * Emits layer_visibility_changed() if there is a change.
 *
 * @param layer The layer to change.
 * @param visible @c true to show the layer, @c false to hide it.
 */
void ViewSettings::set_layer_visible(int layer, bool visible) {

  Q_ASSERT(layer >= min_layer && layer <= max_layer);

  if (visible == is_layer_visible(layer)) {
    return;
  }

  if (visible) {
    visible_layers.insert(layer);
  }
  else {
    visible_layers.erase(layer);
  }
  emit layer_visibility_changed(layer, visible);
}

/**
 * @brief Shows all layers.
 *
 * Emits layers_visibility_changed() for each layers whose visibility changes.
 */
void ViewSettings::show_all_layers() {

  for (int i = min_layer; i <= max_layer; ++i) {
    set_layer_visible(i, true);
  }
}

/**
 * @brief Hides all layers.
 *
 * Emits layers_visibility_changed() for each layers whose visibility changes.
 */
void ViewSettings::hide_all_layers() {

  for (int i = min_layer; i <= max_layer; ++i) {
    set_layer_visible(i, false);
  }
}

/**
 * @brief Returns whether traversable entities are currently shown.
 * @return @c true if traversablers are shown.
 */
bool ViewSettings::are_traversables_visible() const {
  return traversables_visible;
}

/**
 * @brief Shows or hide traversable entities.
 *
 * Emits traversables_visbility_changed() if there is a change.
 *
 * @param traversable_visible @c true to show traversables.
 */
void ViewSettings::set_traversables_visible(bool traversables_visible) {

  if (traversables_visible == this->traversables_visible) {
    return;
  }

  this->traversables_visible = traversables_visible;

  emit traversables_visibility_changed(traversables_visible);
}

/**
 * @brief Returns whether obstacle entities are currently shown.
 * @return @c true if obstaclers are shown.
 */
bool ViewSettings::are_obstacles_visible() const {
  return obstacles_visible;
}

/**
 * @brief Shows or hide obstacle entities.
 *
 * Emits obstacles_visbility_changed() if there is a change.
 *
 * @param obstacle_visible @c true to show obstacles.
 */
void ViewSettings::set_obstacles_visible(bool obstacles_visible) {

  if (obstacles_visible == this->obstacles_visible) {
    return;
  }

  this->obstacles_visible = obstacles_visible;

  emit obstacles_visibility_changed(obstacles_visible);
}
/**
 * @brief Returns whether a entity type is currently visible.
 * @param entity_type The entity type to test.
 * @return @c true if this entity type is visible, @c false if it is hidden.
 */
bool ViewSettings::is_entity_type_visible(EntityType entity_type) const {

  return visible_entity_types.find(entity_type) != visible_entity_types.end();
}

/**
 * @brief Shows or hides an entity type.
 *
 * Emits entity_type_visibility_changed() if there is a change.
 *
 * @param entity_type The entity type to change.
 * @param visible @c true to show the entity type, @c false to hide it.
 */
void ViewSettings::set_entity_type_visible(EntityType entity_type, bool visible) {

  if (visible == is_entity_type_visible(entity_type)) {
    return;
  }

  if (visible) {
    visible_entity_types.insert(entity_type);
  }
  else {
    visible_entity_types.erase(entity_type);
  }
  emit entity_type_visibility_changed(entity_type, visible);
}

/**
 * @brief Shows all entity types.
 *
 * Emits entity_type_visibility_changed() for each type whose visibility changes.
 */
void ViewSettings::show_all_entity_types() {

  const QList<EntityType>& entity_types = EntityTraits::get_values();
  for (EntityType entity_type : entity_types) {
    set_entity_type_visible(entity_type, true);
  }
}

/**
 * @brief Hides all entity types.
 *
 * Emits entity_type_visibility_changed() for each type whose visibility changes.
 */
void ViewSettings::hide_all_entity_types() {

  const QList<EntityType>& entity_types = EntityTraits::get_values();
  for (EntityType entity_type : entity_types) {
    set_entity_type_visible(entity_type, false);
  }
}

}
