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
#ifndef SOLARUSEDITOR_MAP_MODEL_H
#define SOLARUSEDITOR_MAP_MODEL_H

#include <solarus/MapData.h>
#include <QItemSelectionModel>

class Quest;
class TilesetModel;

/**
 * @brief Model that wraps a map.
 *
 * It makes the link between the editor and the map data of the
 * Solarus library.
 * Signals are sent when something changes in the wrapped map.
 * This model also stores the selection information.
 */
class MapModel : public QObject {
  Q_OBJECT

public:

  // Creation.
  MapModel(
      Quest& quest, const QString& map_id, QObject* parent = nullptr);

  Quest& get_quest();
  QString get_map_id() const;

  TilesetModel* get_tileset_model() const;
  QString get_tileset_id() const;
  QString get_music_id() const;

public slots:

  void save() const;

private:

  Quest& quest;                   /**< The quest the tileset belongs to. */
  const QString map_id;           /**< Id of the map. */
  Solarus::MapData map;           /**< Map data wrapped by this model. */
  TilesetModel* tileset_model;    /**< Tileset of this map. nullptr if not set. */

};

#endif
