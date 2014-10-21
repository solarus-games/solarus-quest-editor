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
#ifndef SOLARUSEDITOR_ENUM_TRAITS_H
#define SOLARUSEDITOR_ENUM_TRAITS_H

#include "gui/enum_selector.h"  // To help QtDesigner build selectors using a single include.
#include <QIcon>
#include <QList>
#include <QString>

/**
 * \brief Gives info about enumerated values of a type E.
 *
 * Specializations of this template class should implement at least the
 * following public functions:
 * - static QList<E> get_values();
 * - static QString get_friendly_name(E value);
 * - static QIcon get_icon(E value);
 */
template<typename E>
class EnumTraits {

};

#endif
