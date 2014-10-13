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
#include "color.h"

namespace Color {

/**
 * @brief Converts a Solarus color to a Qt color.
 * @param solarus_color A color for the Solarus library.
 * @return The equivalent QColor.
 */
QColor to_qcolor(const Solarus::Color& solarus_color) {

  uint8_t r, g, b, a;
  solarus_color.get_components(r, g, b, a);
  return QColor(r, g, b, a);
}

/**
 * @brief Converts a Qt color to a Solarus color.
 * @param qt_color A Qt color.
 * @return The equivalent Solarus color.
 */
Solarus::Color to_solarus_color(const QColor& qt_color) {

  return Solarus::Color(
        qt_color.red(), qt_color.green(), qt_color.blue(), qt_color.alpha());
}

}
