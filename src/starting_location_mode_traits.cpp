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
#include "starting_location_mode_traits.h"
#include <QApplication>

namespace SolarusEditor {

/**
 * @brief Returns all values.
 * @return The existing values.
 */
QList<StartingLocationMode> EnumTraits<StartingLocationMode>::get_values() {

  return QList<StartingLocationMode>::fromStdList(
        Solarus::EnumInfo<StartingLocationMode>::enums()
  );
}

/**
 * @brief Returns a user-friendly name describing a value.
 * @param value A value.
 * @return The human-readable name of this value in the current language.
 */
QString EnumTraits<StartingLocationMode>::get_friendly_name(StartingLocationMode value) {

  // Use a switch to ensure we don't forget a value,
  // and also to translate names dynamically.
  switch (value) {

  case StartingLocationMode::WHEN_WORLD_CHANGES:
    return QApplication::tr("When the world changes");

  case StartingLocationMode::YES:
    return QApplication::tr("Always");

  case StartingLocationMode::NO:
    return QApplication::tr("Never");

  }

  return QString();
}

/**
 * @brief Returns an icon representing a value.
 * @param value A value.
 * @return The corresponding icon.
 */
QIcon EnumTraits<StartingLocationMode>::get_icon(StartingLocationMode value) {

  Q_UNUSED(value);
  return QIcon();
}

/**
 * @brief Returns the Lua name of a value.
 * @param value A ground value.
 * @return The corresponding Lua name.
 */
QString EnumTraits<StartingLocationMode>::get_lua_name(StartingLocationMode value) {
  return QString::fromStdString(Solarus::enum_to_name(value));
}

/**
 * @brief Returns a value given its Lua name.
 * @param name A name.
 * @return The corresponding value.
 */
StartingLocationMode EnumTraits<StartingLocationMode>::get_by_lua_name(
    const QString& name) {
  return Solarus::name_to_enum<StartingLocationMode>(name.toStdString());
}

}
