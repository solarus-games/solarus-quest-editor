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
#ifndef SOLARUSEDITOR_TRANSITION_TRAITS_H
#define SOLARUSEDITOR_TRANSITION_TRAITS_H

#include "enum_traits.h"
#include <solarus/Transition.h>

namespace SolarusEditor {

using TransitionStyle = Solarus::Transition::Style;
using TransitionTraits = EnumTraits<TransitionStyle>;

/**
 * \brief Provides useful properties of TransitionStyle enum values.
 */
template<>
class EnumTraits<TransitionStyle> {

public:

  static QList<TransitionStyle> get_values();
  static QString get_friendly_name(TransitionStyle value);
  static QIcon get_icon(TransitionStyle value);

  static QString get_lua_name(TransitionStyle value);
  static TransitionStyle get_by_lua_name(const QString& name);

};

}

#endif
