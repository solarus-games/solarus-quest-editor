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
#include "tile_pattern_animation_traits.h"
#include <QApplication>

/**
 * @brief Returns all values.
 * @return The existing values.
 */
QList<TilePatternAnimation> EnumTraits<TilePatternAnimation>::get_values() {

  return QList<TilePatternAnimation>()
      << TilePatternAnimation::NONE
      << TilePatternAnimation::SEQUENCE_012
      << TilePatternAnimation::SEQUENCE_0121
      << TilePatternAnimation::SELF_SCROLLING
      << TilePatternAnimation::PARALLAX_SCROLLING
      << TilePatternAnimation::SEQUENCE_012_PARALLAX
      << TilePatternAnimation::SEQUENCE_0121_PARALLAX;
}

/**
 * @brief Returns a user-friendly name describing a value.
 * @param value A value.
 * @return The human-readable name of this value in the current language.
 */
QString EnumTraits<TilePatternAnimation>::get_friendly_name(TilePatternAnimation value) {

  // Use a switch to ensure we don't forget a value,
  // and also to translate names dynamically.
  switch (value) {

  case TilePatternAnimation::NONE:
    return QApplication::tr("None", "Tile pattern animation");

  case TilePatternAnimation::SEQUENCE_012:
    return QApplication::tr("Frames 1-2-3-1");

  case TilePatternAnimation::SEQUENCE_0121:
    return QApplication::tr("Frames 1-2-3-2-1");

  case TilePatternAnimation::SELF_SCROLLING:
    return QApplication::tr("Scrolling on itself");

  case TilePatternAnimation::PARALLAX_SCROLLING:
    return QApplication::tr("Parallax scrolling");

  case TilePatternAnimation::SEQUENCE_012_PARALLAX:
    return QApplication::tr("Frames 1-2-3-1, parallax");

  case TilePatternAnimation::SEQUENCE_0121_PARALLAX:
    return QApplication::tr("Frames 1-2-3-2-1, parallax");

  }

  return "";
}

/**
 * @brief Returns an icon representing a value.
 * @param value A value.
 * @return The corresponding icon.
 */
QIcon EnumTraits<TilePatternAnimation>::get_icon(TilePatternAnimation /* value */) {
  // No icon for now.
  return QIcon();
}
