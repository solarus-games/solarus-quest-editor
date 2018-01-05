/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_CLOSABLE_TAB_BAR_H
#define SOLARUSEDITOR_CLOSABLE_TAB_BAR_H

#include <QTabBar>

namespace SolarusEditor {

/**
 * \brief QTabBar whose tabs can be closed with a button and a middle mouse
 * button press event.
 *
 * This class only exists because it is necessary to subclass QTabBar to detect
 * middle mouse button clicks.
 *
 * Tabs are not closed directly: instead, a signal tabCloseRequested is sent.
 */
class ClosableTabBar : public QTabBar {
  Q_OBJECT

public:

  ClosableTabBar(QWidget* parent = nullptr);

protected:

  virtual void mousePressEvent(QMouseEvent* event) override;

};

}

#endif
