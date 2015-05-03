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
#include "grid_style.h"
#include <QApplication>

/**
 * @brief Returns all values.
 * @return The existing values.
 */
QList<GridStyle> GridStyleTraits::get_values() {

  return QList<GridStyle>()
    << GridStyle::PLAIN
    << GridStyle::DASHED
    << GridStyle::INTERSECT_CROSS
    << GridStyle::INTERSECT_POINT;
}

/**
 * @brief Returns a user-friendly name describing a value.
 * @param value A value.
 * @return The human-readable name of this value in the current language.
 */
QString GridStyleTraits::get_friendly_name(GridStyle value) {

  switch (value) {

  case GridStyle::PLAIN:
    return QApplication::tr("Plain");

  case GridStyle::DASHED:
    return QApplication::tr("Dashed");

  case GridStyle::INTERSECT_CROSS:
    return QApplication::tr("Intersections (cross)");

  case GridStyle::INTERSECT_POINT:
    return QApplication::tr("Intersections (point)");
  }

  return "";
}

/**
 * @brief Returns an icon representing a value.
 * @param value A value.
 * @return The corresponding icon.
 */
QIcon GridStyleTraits::get_icon(GridStyle /*value*/) {

  return QIcon();
}
