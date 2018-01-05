/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_PATTERN_REPEAT_MODE_TRAITS_H
#define SOLARUSEDITOR_PATTERN_REPEAT_MODE_TRAITS_H

#include "enum_traits.h"
#include <solarus/entities/TilesetData.h>

namespace SolarusEditor {

using TilePatternRepeatMode = Solarus::TilePatternRepeatMode;
using TilePatternRepeatModeTraits = EnumTraits<TilePatternRepeatMode>;

/**
 * \brief Provides useful properties of tile pattern repeat mode enum values.
 */
template<>
class EnumTraits<TilePatternRepeatMode> {

public:

  static QList<TilePatternRepeatMode> get_values();
  static QString get_friendly_name(TilePatternRepeatMode value);
  static QIcon get_icon(TilePatternRepeatMode value);

  static QString get_lua_name(TilePatternRepeatMode value);
  static TilePatternRepeatMode get_by_lua_name(const QString& name);

};

}

#endif
