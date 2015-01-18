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

  QSize get_size() const;
  void set_size(const QSize& size);
  bool has_world() const;
  QString get_world() const;
  void set_world(const QString& world);
  bool has_floor() const;
  int get_floor() const;
  void set_floor(int floor);
  QPoint get_location() const;
  void set_location(const QPoint& location);
  TilesetModel* get_tileset_model() const;
  QString get_tileset_id() const;
  void set_tileset_id(const QString& tileset_id);
  QString get_music_id() const;
  void set_music_id(const QString& music_id);

  static constexpr int NO_FLOOR = Solarus::MapData::NO_FLOOR;

signals:

  void size_changed(const QSize& size);
  void world_changed(const QString& world);
  void floor_changed(int floor);
  void location_changed(const QPoint& location);
  void tileset_id_changed(const QString& tileset_id);
  void music_id_changed(const QString& music_id);

public slots:

  void save() const;

private:

  Quest& quest;                   /**< The quest the tileset belongs to. */
  const QString map_id;           /**< Id of the map. */
  Solarus::MapData map;           /**< Map data wrapped by this model. */
  TilesetModel* tileset_model;    /**< Tileset of this map. nullptr if not set. */

};

#endif
