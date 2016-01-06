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
#ifndef SOLARUSEDITOR_PATTERN_ANIMATION_TRAITS_H
#define SOLARUSEDITOR_PATTERN_ANIMATION_TRAITS_H

#include "enum_traits.h"
#include "pattern_animation.h"
#include <solarus/entities/TilesetData.h>

using PatternAnimationTraits = EnumTraits<PatternAnimation>;

/**
 * \brief Provides useful properties of tile pattern animation enum values.
 */
template<>
class EnumTraits<PatternAnimation> {

public:

  static QList<PatternAnimation> get_values();
  static QString get_friendly_name(PatternAnimation value);
  static QIcon get_icon(PatternAnimation value);

  static bool is_multi_frame(PatternAnimation value);
  static int get_num_frames(PatternAnimation value);
  static Solarus::TileScrolling get_scrolling(PatternAnimation value);

};

#endif
