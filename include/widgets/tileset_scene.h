/*
 * Copyright (C) 2014-2017 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_TILESET_SCENE_H
#define SOLARUSEDITOR_TILESET_SCENE_H

#include <QGraphicsScene>

class QItemSelection;

namespace SolarusEditor {

class PatternItem;
class Quest;
class TilesetModel;

/**
 * @brief The scene containing all patterns in the tileset main view.
 */
class TilesetScene : public QGraphicsScene {
  Q_OBJECT

public:

  TilesetScene(TilesetModel& model, QObject* parent);

  const TilesetModel& get_model() const;
  const Quest& get_quest() const;

  int get_pattern_index(const QGraphicsItem* item);

  void select_all();
  void unselect_all();

protected:

  void drawBackground(QPainter* painter, const QRectF& rect) override;

private slots:

  void update_selection_to_scene(
      const QItemSelection& selected, const QItemSelection& deselected);
  void set_selection_from_scene();
  void update_pattern_position(int index);
  void update_pattern_animation(int index);
  void pattern_created(int new_index, const QString& new_id);
  void pattern_deleted(int old_index, const QString& old_id);
  void pattern_id_changed(int old_index, const QString& old_id,
                          int new_index, const QString& new_id);
  void image_changed();

private:

  void build();

  TilesetModel& model;            /**< The tileset represented. */
  QList<PatternItem*>
      pattern_items;              /**< Each pattern item in the scene,
                                   * ordered as in the model. */

};

}

#endif
