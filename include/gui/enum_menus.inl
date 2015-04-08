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
#include "enum_traits.h"
#include <QMenu>

template<typename E>
class EnumTraits;

/**
 * @brief Creates a menu with all values of the enum.
 * @param checkable How checkable actions should be.
 * @return The created menu. It has no parent.
 */
template<typename E>
QMenu* EnumMenus<E>::create_menu(EnumMenuCheckableOption checkable) {

  QMenu* menu = new QMenu();
  QList<QAction*> actions = create_actions(*menu, checkable);
  for (QAction* action : actions) {
    menu->addAction(action);
  }
  return menu;
}

/**
 * @brief Creates actions corresponding to all values of the enum.
 *
 * The data() of each action is the enum value converted to an integer.
 *
 * @param parent Parent of actions to create. They will be added to this widget.
 * @param checkable How checkable actions should be.
 * @param on_triggered Optional function to call when an action is triggered.
 * The function takes the enum value as parameter.
 * @return The created actions. They have no parent.
 */
template<typename E>
QList<QAction*> EnumMenus<E>::create_actions(
    QWidget& parent,
    EnumMenuCheckableOption checkable,
    std::function<void (const E&)> on_triggered) {

  QObject* action_parent = &parent;

  // Create a group if actions should be exclusive.
  QActionGroup* group = nullptr;
  if (checkable == EnumMenuCheckableOption::CHECKABLE_EXCLUSIVE) {
    group = new QActionGroup(&parent);
    action_parent = group;
  }

  // Create the actions.
  QList<QAction*> actions;
  for (const E& value : EnumTraits<E>::get_values()) {
    const QIcon& icon = EnumTraits<E>::get_icon(value);
    const QString& text = EnumTraits<E>::get_friendly_name(value);
    QAction* action = new QAction(icon, text, action_parent);
    action->setData(static_cast<int>(value));
    if (checkable != EnumMenuCheckableOption::NON_CHECKABLE) {
      action->setCheckable(true);
    }

    // Connect to the provided callback if any.
    if (on_triggered) {
      QObject::connect(action, &QAction::triggered, [=] {
        on_triggered(value);
      });
    }

    actions << action;
  }

  parent.addActions(actions);

  return actions;
}
