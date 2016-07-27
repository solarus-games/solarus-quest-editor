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
#include "ground_traits.h"
#include <solarus/entities/GroundInfo.h>
#include <QApplication>

namespace SolarusEditor {

/**
 * @brief Returns all values.
 * @return The existing values.
 */
QList<Ground> EnumTraits<Ground>::get_values() {

  return QList<Ground>::fromStdList(
        Solarus::EnumInfo<Ground>::enums()
  );
}

/**
 * @brief Returns a user-friendly name describing a value.
 * @param value A value.
 * @return The human-readable name of this value in the current language.
 */
QString EnumTraits<Ground>::get_friendly_name(Ground value) {

  // Use a switch to ensure we don't forget a value,
  // and also to translate names dynamically.
  switch (value) {

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

/**
 * @brief Returns an icon representing a value.
 * @param value A value.
 * @return The corresponding icon.
 */
QIcon EnumTraits<Ground>::get_icon(Ground value) {
  return QIcon(":/images/ground_" + get_lua_name(value) + ".png");
}

/**
 * @brief Returns the Lua name of a value.
 * @param value A ground value.
 * @return The corresponding Lua name.
 */
QString EnumTraits<Ground>::get_lua_name(Ground value) {
  return QString::fromStdString(Solarus::enum_to_name(value));
}

/**
 * @brief Returns a value given its Lua name.
 * @param name A name.
 * @return The corresponding value.
 */
Ground EnumTraits<Ground>::get_by_lua_name(
    const QString& name) {
  return Solarus::name_to_enum<Ground>(name.toStdString());
}

/**
 * @brief Returns whether a ground is traversable or obstacle.
 *
 * This is a simplification of the various possible grounds.
 *
 * @param value A ground value.
 * @return @c true if is traversable, @c false if it obstacle.
 */
bool EnumTraits<Ground>::is_traversable(Ground value) {

  switch (value) {

  case Ground::EMPTY:
  case Ground::TRAVERSABLE:
  case Ground::DEEP_WATER:
  case Ground::SHALLOW_WATER:
  case Ground::GRASS:
  case Ground::HOLE:
  case Ground::LAVA:
  case Ground::PRICKLE:
  case Ground::ICE:
  case Ground::LADDER:
    return true;

  case Ground::WALL:
  case Ground::LOW_WALL:
  case Ground::WALL_TOP_RIGHT:
  case Ground::WALL_TOP_LEFT:
  case Ground::WALL_BOTTOM_LEFT:
  case Ground::WALL_BOTTOM_RIGHT:
  case Ground::WALL_TOP_RIGHT_WATER:
  case Ground::WALL_TOP_LEFT_WATER:
  case Ground::WALL_BOTTOM_LEFT_WATER:
  case Ground::WALL_BOTTOM_RIGHT_WATER:
    return false;
  }

  return true;
}

}
