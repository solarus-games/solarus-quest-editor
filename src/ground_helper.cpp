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
#include "ground_helper.h"
#include <QApplication>

namespace GroundHelper {

/**
 * @brief Returns a user-friendly name for the specified ground value.
 * @param ground A type of terrain.
 * @return The human-readable name of this ground in the current language.
 */
QString get_ground_friendly_name(Ground ground) {

  // Use a switch to ensure we don't forget a value,
  // and also to translate names dynamically.
  switch (ground) {

  case Ground::EMPTY:
    return QApplication::tr("Empty");

  case Ground::TRAVERSABLE:
    return QApplication::tr("Traversable");

  case Ground::WALL:
    return QApplication::tr("Wall");

  case Ground::LOW_WALL:
    return QApplication::tr("Low wall");

  case Ground::WALL_TOP_RIGHT:
    return QApplication::tr("Top right");

  case Ground::WALL_TOP_LEFT:
    return QApplication::tr("Top left");

  case Ground::WALL_BOTTOM_LEFT:
    return QApplication::tr("Bottom left");

  case Ground::WALL_BOTTOM_RIGHT:
    return QApplication::tr("Bottom right");

  case Ground::WALL_TOP_RIGHT_WATER:
    return QApplication::tr("Top right (water)");

  case Ground::WALL_TOP_LEFT_WATER:
    return QApplication::tr("Top left (water)");

  case Ground::WALL_BOTTOM_LEFT_WATER:
    return QApplication::tr("Bottom left (water)");

  case Ground::WALL_BOTTOM_RIGHT_WATER:
    return QApplication::tr("Bottom right (water)");

  case Ground::DEEP_WATER:
    return QApplication::tr("Deep water");

  case Ground::SHALLOW_WATER:
    return QApplication::tr("Shallow water");

  case Ground::GRASS:
    return QApplication::tr("Grass");

  case Ground::HOLE:
    return QApplication::tr("Hole");

  case Ground::ICE:
    return QApplication::tr("Ice");

  case Ground::LADDER:
    return QApplication::tr("Ladder");

  case Ground::PRICKLE:
    return QApplication::tr("Prickles");

  case Ground::LAVA:
    return QApplication::tr("Lava");

  }

  return "";
}

}
