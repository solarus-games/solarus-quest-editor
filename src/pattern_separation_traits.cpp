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
#include "pattern_separation_traits.h"
#include <QApplication>

/**
 * @brief Returns all values.
 * @return The existing values.
 */
QList<PatternSeparation> EnumTraits<PatternSeparation>::get_values() {

  return QList<PatternSeparation>()
      << PatternSeparation::HORIZONTAL
      << PatternSeparation::VERTICAL;
}

/**
 * @brief Returns a user-friendly name describing a value.
 * @param value A value.
 * @return The human-readable name of this value in the current language.
 */
QString EnumTraits<PatternSeparation>::get_friendly_name(PatternSeparation value) {

  switch (value) {

  case PatternSeparation::HORIZONTAL:
    return QApplication::tr("Horizontal");

  case PatternSeparation::VERTICAL:
    return QApplication::tr("Vertical");

  }

  return "";
}

/**
 * @brief Returns an icon representing a value.
 * @param value A value.
 * @return The corresponding icon.
 */
QIcon EnumTraits<PatternSeparation>::get_icon(PatternSeparation value) {

  switch (value) {

  case PatternSeparation::HORIZONTAL:
    return QIcon(":/images/animation_separation_horizontal.png");

  case PatternSeparation::VERTICAL:
    return QIcon(":/images/animation_separation_vertical.png");

  }

  return QIcon();
}
