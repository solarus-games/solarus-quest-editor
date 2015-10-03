/*
 * Copyright (C) 2014-2015 Christopho, Sorus - http://www.solarus-games.org
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
#include "transition_traits.h"
#include <QApplication>

/**
 * @brief Returns all values.
 * @return The existing values.
 */
QList<TransitionStyle> EnumTraits<TransitionStyle>::get_values() {

  return QList<TransitionStyle>()
      << TransitionStyle::IMMEDIATE
      << TransitionStyle::FADE
      << TransitionStyle::SCROLLING;
}

/**
 * @brief Returns a user-friendly name describing a value.
 * @param value A value.
 * @return The human-readable name of this value in the current language.
 */
QString EnumTraits<TransitionStyle>::get_friendly_name(TransitionStyle value) {

  switch (value) {

  case TransitionStyle::IMMEDIATE:
    return QApplication::tr("Immediate");

  case TransitionStyle::FADE:
    return QApplication::tr("Fade in/out");

  case TransitionStyle::SCROLLING:
    return QApplication::tr("Scrolling");

  }

  return "";
}

/**
 * @brief Returns an icon representing a value.
 * @param value A value.
 * @return The corresponding icon.
 */
QIcon EnumTraits<TransitionStyle>::get_icon(TransitionStyle value) {

  Q_UNUSED(value)
  // No icon for now.
  return QIcon();
}

/**
 * @brief Returns the Lua name of a value.
 * @param value A value.
 * @return The corresponding Lua name.
 */
QString EnumTraits<TransitionStyle>::get_lua_name(TransitionStyle value) {
  return QString::fromStdString(Solarus::enum_to_name(value));
}

/**
 * @brief Returns a value given its Lua name.
 * @param name A name.
 * @return The corresponding value.
 */
TransitionStyle EnumTraits<TransitionStyle>::get_by_lua_name(
    const QString& name) {
  return Solarus::name_to_enum<TransitionStyle>(name.toStdString());
}
