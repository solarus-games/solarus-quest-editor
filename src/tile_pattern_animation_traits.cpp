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

/**
 * @brief Returns whether a pattern is a multi-frame pattern.
 * @param index A pattern index.
 * @return @c true if this is a multi-frame pattern.
 * Returns @c falseif the pattern does not exist.
 */
bool EnumTraits<TilePatternAnimation>::is_multi_frame(TilePatternAnimation value) {

  return get_num_frames(value) > 1;
}

/**
 * @brief Returns the number of frames of an animation type.
 * @param value A pattern animation.
 * @return The number of frames (1, 3 or 4).
 */
int EnumTraits<TilePatternAnimation>::get_num_frames(TilePatternAnimation value) {

  switch (value) {

  case TilePatternAnimation::NONE:
  case TilePatternAnimation::SELF_SCROLLING:
  case TilePatternAnimation::PARALLAX_SCROLLING:
    return 1;

  case TilePatternAnimation::SEQUENCE_012:
  case TilePatternAnimation::SEQUENCE_012_PARALLAX:
    return 2;

  case TilePatternAnimation::SEQUENCE_0121:
  case TilePatternAnimation::SEQUENCE_0121_PARALLAX:
    return 3;
  }

  return 1;
}

/**
 * @brief Returns the scrolling property of an animation type.
 * @param value A pattern animation.
 * @return The scrolling property.
 */
Solarus::TileScrolling EnumTraits<TilePatternAnimation>::get_scrolling(TilePatternAnimation value) {

  switch (value) {

  case TilePatternAnimation::NONE:
  case TilePatternAnimation::SEQUENCE_012:
  case TilePatternAnimation::SEQUENCE_0121:
    return Solarus::TileScrolling::NONE;

  case TilePatternAnimation::SELF_SCROLLING:
    return Solarus::TileScrolling::SELF;

  case TilePatternAnimation::PARALLAX_SCROLLING:
  case TilePatternAnimation::SEQUENCE_012_PARALLAX:
  case TilePatternAnimation::SEQUENCE_0121_PARALLAX:
    return Solarus::TileScrolling::PARALLAX;

  }

  return Solarus::TileScrolling::NONE;
}
