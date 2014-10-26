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
#include "rectangle.h"

namespace Rectangle {

/**
 * @brief Converts a Solarus rectangle to a Qt rectangle.
 * @param solarus_rect A rectangle for the Solarus library.
 * @return The equivalent QRect.
 */
QRect to_qrect(const Solarus::Rectangle& solarus_rect) {

  return QRect(solarus_rect.get_x(), solarus_rect.get_y(),
               solarus_rect.get_width(), solarus_rect.get_height());
}

/**
 * @brief Converts a Qt rectangle to a Solarus rectangle.
 * @param qt_rect A Qt rectangle.
 * @return The equivalent Solarus rectangle.
 */
Solarus::Rectangle to_solarus_rect(const QRect& qt_rect) {

  return Solarus::Rectangle(
        qt_rect.x(), qt_rect.y(), qt_rect.width(), qt_rect.height());
}

}
