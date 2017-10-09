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
#ifndef SOLARUSEDITOR_STARTING_LOCATION_MODE_TRAITS_H
#define SOLARUSEDITOR_STARTING_LOCATION_MODE_TRAITS_H

#include "enum_traits.h"
#include <solarus/entities/StartingLocationMode.h>

namespace SolarusEditor {

using StartingLocationMode = Solarus::StartingLocationMode;
using StartingLocationModeTraits = EnumTraits<StartingLocationMode>;

/**
 * \brief Provides useful properties of starting location mode enum values.
 */
template<>
class EnumTraits<StartingLocationMode> {

public:

  static QList<StartingLocationMode> get_values();
  static QString get_friendly_name(StartingLocationMode value);
  static QIcon get_icon(StartingLocationMode value);

  static QString get_lua_name(StartingLocationMode value);
  static StartingLocationMode get_by_lua_name(const QString& name);

};

}

#endif
