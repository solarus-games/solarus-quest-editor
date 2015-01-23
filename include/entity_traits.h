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
#ifndef SOLARUSEDITOR_ENTITY_TRAITS_H
#define SOLARUSEDITOR_ENTITY_TRAITS_H

#include "enum_traits.h"
#include <solarus/entities/EntityType.h>

using EntityType = Solarus::EntityType;
using EntityTraits = EnumTraits<EntityType>;

/**
 * \brief Provides useful properties of entity type enum values.
 */
template<>
class EnumTraits<EntityType> {

public:

  static QList<EntityType> get_values();
  static QString get_friendly_name(EntityType value);
  static QIcon get_icon(EntityType value);

  static QString get_lua_name(EntityType value);

  static bool can_be_stored_in_map_file(EntityType type);

};

#endif
