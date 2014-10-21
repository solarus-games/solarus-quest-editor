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
#include "layer_traits.h"
#include <QApplication>

/**
 * @brief Returns all values.
 * @return The existing values.
 */
QList<Layer> EnumTraits<Layer>::get_values() {

  return QList<Layer>()
      << Solarus::LAYER_LOW
      << Solarus::LAYER_INTERMEDIATE
      << Solarus::LAYER_HIGH;
}

/**
 * @brief Returns a user-friendly name describing a value.
 * @param value A value.
 * @return The human-readable name of this value in the current language.
 */
QString EnumTraits<Layer>::get_friendly_name(Layer value) {

  switch (value) {

  case Solarus::LAYER_LOW:
    return QApplication::tr("Low", "Layer");

  case Solarus::LAYER_INTERMEDIATE:
    return QApplication::tr("Intermediate", "Layer");

  case Solarus::LAYER_HIGH:
    return QApplication::tr("High", "Layer");

  case Solarus::LAYER_NB:
    return "";

  }

  return "";
}

/**
 * @brief Returns an icon representing a value.
 * @param value A value.
 * @return The corresponding icon.
 */
QIcon EnumTraits<Layer>::get_icon(Layer /* value */) {
  // No icon for now.
  return QIcon();
}
