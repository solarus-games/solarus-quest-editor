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
#ifndef SOLARUSEDITOR_ENUM_MENUS_H
#define SOLARUSEDITOR_ENUM_MENUS_H

#include <QList>
#include <functional>

class QAction;
class QWidget;
class QMenu;

/**
 * @brief Whether actions should be non-checkable, checkable or mutually exclusive.
 */
enum class EnumMenuCheckableOption {
  NON_CHECKABLE,
  CHECKABLE,
  CHECKABLE_EXCLUSIVE,
};

/**
 * @brief Helps creating actions and menus from the values of an enum.
 */
template<typename E>
class EnumMenus {

public:

  static QMenu* create_menu(EnumMenuCheckableOption checkable);

  static QList<QAction*> create_actions(
      QWidget& parent,
      EnumMenuCheckableOption checkable,
      std::function<void (const E&)> on_triggered = std::function<void (const E&)>());

  // TODO create_toolbar()
};

#include "gui/enum_menus.inl"

#endif
