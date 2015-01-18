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
#include "size.h"

namespace Size {

/**
 * @brief Converts a Solarus size to a Qt size.
 * @param solarus_size A size for the Solarus library.
 * @return The equivalent QRect.
 */
QSize to_qsize(const Solarus::Size& solarus_size) {

  return QSize(solarus_size.width, solarus_size.height);
}

/**
 * @brief Converts a Qt size to a Solarus size.
 * @param qt_size A Qt size.
 * @return The equivalent Solarus size.
 */
Solarus::Size to_solarus_size(const QSize& qt_size) {

  return Solarus::Size(qt_size.width(), qt_size.height());
}

}
