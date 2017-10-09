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
#include "border_kind_traits.h"
#include <QApplication>

namespace SolarusEditor {

namespace {

const QList<BorderKind> names = QList<BorderKind>()
    << BorderKind::NONE
    << BorderKind::RIGHT
    << BorderKind::TOP
    << BorderKind::LEFT
    << BorderKind::BOTTOM
    << BorderKind::TOP_RIGHT_CONVEX
    << BorderKind::TOP_LEFT_CONVEX
    << BorderKind::BOTTOM_LEFT_CONVEX
    << BorderKind::BOTTOM_RIGHT_CONVEX
    << BorderKind::TOP_RIGHT_CONCAVE
    << BorderKind::TOP_LEFT_CONCAVE
    << BorderKind::BOTTOM_LEFT_CONCAVE
    << BorderKind::BOTTOM_RIGHT_CONCAVE;

}  // Anonymous namespace.

/**
 * @brief Returns all values.
 * @return The existing values.
 */
QList<BorderKind> EnumTraits<BorderKind>::get_values() {
  return names;
}

/**
 * @brief Returns a user-friendly name describing a value.
 * @param value A value.
 * @return The human-readable name of this value in the current language.
 */
QString EnumTraits<BorderKind>::get_friendly_name(BorderKind value) {

  // Use a switch to ensure we don't forget a value,
  // and also to translate names dynamically.
  switch (value) {

  case BorderKind::NONE:
    return QApplication::tr("None");

  case BorderKind::RIGHT:
    return QApplication::tr("Right side");

  case BorderKind::TOP:
    return QApplication::tr("Top side");

  case BorderKind::LEFT:
    return QApplication::tr("Left side");

  case BorderKind::BOTTOM:
    return QApplication::tr("Bottom side");

  case BorderKind::TOP_RIGHT_CONVEX:
    return QApplication::tr("Top-right corner (convex)");

  case BorderKind::TOP_LEFT_CONVEX:
    return QApplication::tr("Top-left corner (convex)");

  case BorderKind::BOTTOM_LEFT_CONVEX:
    return QApplication::tr("Bottom-left corner (convex)");

  case BorderKind::BOTTOM_RIGHT_CONVEX:
    return QApplication::tr("Bottom-right corner (convex)");

  case BorderKind::TOP_RIGHT_CONCAVE:
    return QApplication::tr("Top-right corner (concave)");

  case BorderKind::TOP_LEFT_CONCAVE:
    return QApplication::tr("Top-left corner (concave)");

  case BorderKind::BOTTOM_LEFT_CONCAVE:
    return QApplication::tr("Bottom-left corner (concave)");

  case BorderKind::BOTTOM_RIGHT_CONCAVE:
    return QApplication::tr("Bottom-right corner (concave)");

  }

  return "";
}

/**
 * @brief Returns an icon representing a value.
 * @param value A value.
 * @return The corresponding icon.
 */
QIcon EnumTraits<BorderKind>::get_icon(BorderKind value) {
  return QIcon(QString(":/images/icon_border_kind_%1.png").arg(static_cast<int>(value)));
}

}
