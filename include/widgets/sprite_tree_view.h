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
#ifndef SOLARUSEDITOR_SPRITE_TREE_VIEW_H
#define SOLARUSEDITOR_SPRITE_TREE_VIEW_H

#include <QTreeView>

namespace SolarusEditor {

class SpriteModel;

/**
 * @brief Tree view of a sprite.
 *
 * This tree view shows all animations and directions of the sprite.
 */
class SpriteTreeView : public QTreeView {
  Q_OBJECT

public:

  SpriteTreeView(QWidget* parent = nullptr);

  void set_model(SpriteModel* sprite);

signals:

  void create_animation_requested();
  void create_direction_requested();
  void rename_animation_requested();
  void duplicate_requested();
  void delete_requested();

protected:

  void contextMenuEvent(QContextMenuEvent* event) override;

private:

  SpriteModel* model;                 /**< The sprite model. */
  QAction* create_animation_action;   /**< Action of adding new animation. */
  QAction* create_direction_action;   /**< Action of adding new direction. */
  QAction* rename_animation_action;   /**< Action of renaming animation. */
  QAction* duplicate_action;          /**< Action of duplicating. */
  QAction* delete_action;             /**< Action of deleting. */

};

}

#endif
