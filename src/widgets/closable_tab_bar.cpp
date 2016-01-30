/*
 * Copyright (C) 2014-2016 Christopho, Solarus - http://www.solarus-games.org
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
#include "widgets/closable_tab_bar.h"
#include <QMouseEvent>

/**
 * @brief Creates a closable tab bar.
 * @param parent The parent object or nullptr.
 */
ClosableTabBar::ClosableTabBar(QWidget* parent) :
  QTabBar(parent) {

  setTabsClosable(true);
}

/**
 * @brief Receives a mouse press event.
 * @param event The event to handle.
 */
void ClosableTabBar::mousePressEvent(QMouseEvent* event) {

  if (event->button() == Qt::MidButton) {
    int index = tabAt(event->pos());
    if (index != -1) {
      // Middle mouse button on a tab: close it.
      emit tabCloseRequested(index);
      return;
    }
  }

  // Default mouse press behavior otherwise.
  QTabBar::mousePressEvent(event);
}
