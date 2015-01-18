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
 * @brief Returns the id of the tileset of this map.
 * @return The tileset id. Returns an empty string if no tileset is set.
 */
QString MapModel::get_tileset_id() const {
  return QString::fromStdString(map.get_tileset_id());
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
