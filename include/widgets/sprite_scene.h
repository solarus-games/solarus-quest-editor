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
#ifndef SOLARUSEDITOR_SPRITE_SCENE_H
#define SOLARUSEDITOR_SPRITE_SCENE_H

#include <QGraphicsScene>
#include "sprite_model.h"

namespace SolarusEditor {

class DirectionItem;

/**
 * @brief The scene containing all directions of the selected animation in
 * sprite main view.
 */
class SpriteScene : public QGraphicsScene {
  Q_OBJECT

  /** Internal sprite model index redefinition
   * (to connect SpriteModel signals to slots correctly). */
  typedef SpriteModel::Index Index;

public:

  SpriteScene(SpriteModel& model, QObject* parent);

  const SpriteModel& get_model() const;
  const Quest& get_quest() const;

  SpriteModel::Index get_direction_index(const QGraphicsItem* item);

protected:

  virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

private slots:

  void update_image();
  void update_selection_to_scene(
      const QItemSelection& selected, const QItemSelection& deselected);
  void set_selection_from_scene();
  void update_direction_position(const Index& index, const QPoint& pos);
  void update_direction_rect(const Index& index);

private:

  void rebuild();

  SpriteModel& model;                 /**< The sprite represented. */
  QString animation_name;             /**< The name of animation currently
                                       * represented. */
  QList<DirectionItem*>
      direction_items;                /**< Each direction item in the scene,
                                       * ordered as in the model. */

  QGraphicsTextItem* missing_text;    /**< Text displayed when the
                                       * source image is missing. */

};

}

#endif
