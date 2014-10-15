/*
 * Copyright (C) 2014 Christopho, Solarus - http://www.solarus-games.org
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

class Quest;
class TilesetModel;
class QGraphicsPixmapItem;
class QItemSelection;

/**
 * @brief The scene containing all patterns in the tileset main view.
 */
class TilesetScene : public QGraphicsScene {
  Q_OBJECT

public:

  TilesetScene(TilesetModel& model, QObject* parent);

  const TilesetModel& get_model() const;
  const Quest& get_quest() const;

protected:

  virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

private slots:

  void update_selection_to_scene(
      const QItemSelection& selected, const QItemSelection& deselected);
  void set_selection_from_scene();

private:

  void build();

  TilesetModel& model;            /**< The tileset represented. */
  QList<QGraphicsPixmapItem*>
      pattern_items;              /**< Each pattern item in the scene,
                                   * ordered as in the model. */

};

#endif
