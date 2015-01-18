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
#include "editor_exception.h"
#include "map_model.h"
#include "quest.h"
#include "point.h"
#include "size.h"
#include "tileset_model.h"
#include <QIcon>

/**
 * @brief Creates a map model.
 * @param quest The quest.
 * @param map_id Id of the map to manage.
 * @param parent The parent object or nullptr.
 * @throws EditorException If the file could not be opened.
 */
MapModel::MapModel(
    Quest& quest,
    const QString& map_id,
    QObject* parent) :
  QObject(parent),
  quest(quest),
  map_id(map_id),
  tileset_model(nullptr) {

  // Load the map data file.
  QString path = quest.get_map_data_file_path(map_id);

  if (!map.import_from_file(path.toStdString())) {
    throw EditorException(tr("Cannot open map data file '%1'").arg(path));
  }

  // Create the tileset object.
  QString tileset_id = get_tileset_id();
  if (!tileset_id.isEmpty()) {
    tileset_model = new TilesetModel(quest, tileset_id, this);
  }

}

/**
 * @brief Returns the quest.
 */
Quest& MapModel::get_quest() {
  return quest;
}

/**
 * @brief Returns the id of the map managed by this model.
 * @return The map id.
 */
QString MapModel::get_map_id() const {
  return map_id;
}

/**
 * @brief Saves the map to its data file.
 * @throws EditorException If the file could not be saved.
 */
void MapModel::save() const {

  QString path = quest.get_map_data_file_path(map_id);

  if (!map.export_to_file(path.toStdString())) {
    throw EditorException(tr("Cannot save map data file '%1'").arg(path));
  }
}

/**
 * @brief Returns the size of the map.
 * @return The size of the map in pixels.
 */
QSize MapModel::get_size() const {

  return Size::to_qsize(map.get_size());
}

/**
 * @brief Sets the size of this map.
 *
 * Emits size_changed() if there is a change.
 *
 * @param size The size to set.
 */
void MapModel::set_size(const QSize& size) {

  const Solarus::Size solarus_size = Size::to_solarus_size(size);
  if (solarus_size == map.get_size()) {
    return;
  }

  map.set_size(solarus_size);
  emit size_changed(size);
}

/**
 * @brief Returns whether a world is set.
 * @return @c true if this map is in a world.
 */
bool MapModel::has_world() const {

  return map.has_world();
}

/**
 * @brief Returns the world this maps belongs to.
 * @return The world name or an empty string.
 */
QString MapModel::get_world() const {

  return QString::fromStdString(map.get_world());
}

/**
 * @brief Sets the world of this map.
 *
 * Emits world_changed() if there is a change.
 *
 * @param world The world name or an empty string.
 */
void MapModel::set_world(const QString& world) {

  const std::string& std_world = world.toStdString();
  if (std_world == map.get_world()) {
    return;
  }

  map.set_world(std_world);
  emit world_changed(world);
}

/**
 * @brief Returns whether a floor is set.
 * @return @c true if this map has a floor value.
 */
bool MapModel::has_floor() const {

  return map.has_floor();
}

/**
 * @brief Returns the floor of this map.
 * @return The floor or NO_FLOOR.
 */
int MapModel::get_floor() const {

  return map.get_floor();
}

/**
 * @brief Sets the floor of this map.
 *
 * Emits floor_changed() if there is a change.
 *
 * @param floor The floor or NO_FLOOR.
 */
void MapModel::set_floor(int floor) {

  if (floor == map.get_floor()) {
    return;
  }

  map.set_floor(floor);
  emit floor_changed(floor);
}

/**
 * @brief Returns the location of this map in its world.
 * @return The location.
 */
QPoint MapModel::get_location() const {

  return Point::to_qpoint(map.get_location());
}

/**
 * @brief Sets the location of this map in its world.
 *
 * Emits location_changed() if there is a change.
 *
 * @return The location.
 */
void MapModel::set_location(const QPoint& location) {

  const Solarus::Point solarus_location = Point::to_solarus_point(location);
  if (solarus_location == map.get_location()) {
    return;
  }

  map.set_location(solarus_location);
  emit location_changed(location);
}

/**
 * @brief Returns the id of the tileset of this map.
 * @return The tileset id. Returns an empty string if no tileset is set.
 */
QString MapModel::get_tileset_id() const {
  return QString::fromStdString(map.get_tileset_id());
}

/**
 * @brief Sets the tileset of this map.
 *
 * Emits tileset_id_changed() if there is a change.
 *
 * @param tileset_id The tileset id.
 */
void MapModel::set_tileset_id(const QString& tileset_id) {

  const std::string std_tileset_id = tileset_id.toStdString();
  if (std_tileset_id == map.get_tileset_id()) {
    return;
  }
  map.set_tileset_id(std_tileset_id);
  emit tileset_id_changed(tileset_id);
}

/**
 * @brief Returns the tileset of this map.
 * @return The tileset. Returns nullptr if no tileset is set.
 */
TilesetModel* MapModel::get_tileset_model() const {
  return tileset_model;
}

/**
 * @brief Returns the id of the music of this map.
 * @return The music id or "none" or "same".
 */
QString MapModel::get_music_id() const {
  return QString::fromStdString(map.get_music_id());
}

/**
 * @brief Sets the music of this map.
 *
 * Emits music_id_changed() if there is a change.
 *
 * @param music_id The music id or "none" or "same".
 */
void MapModel::set_music_id(const QString& music_id) {

  const std::string std_music_id = music_id.toStdString();
  if (std_music_id == map.get_music_id()) {
    return;
  }
  map.set_music_id(std_music_id);
  emit music_id_changed(music_id);
}
