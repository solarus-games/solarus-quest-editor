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
#ifndef SOLARUSEDITOR_SPRITE_TREE_VIEW_H
#define SOLARUSEDITOR_SPRITE_TREE_VIEW_H

#include <QTreeView>

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

  void set_model(SpriteModel& sprite);

signals:

};

#endif
