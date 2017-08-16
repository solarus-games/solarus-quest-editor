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
#ifndef SOLARUSEDITOR_MOUSE_COORDINATES_TRACKING_TOOL_H
#define SOLARUSEDITOR_MOUSE_COORDINATES_TRACKING_TOOL_H

#include <QObject>

class QAbstractScrollArea;

namespace SolarusEditor {

/**
 * @brief Notifies a scrolling widget of mouse movements.
 *
 * The controlled scrolling widget must have signals or slots
 * mouse_coordinates_changed(const QPoint& xy)
 * and mouse_left();
 * The indicated coordinates are relative to the current scrollbar position.
 */
class MouseCoordinatesTrackingTool : public QObject {
  Q_OBJECT

public:

  MouseCoordinatesTrackingTool(QAbstractScrollArea* widget);

  bool eventFilter(QObject* object, QEvent* event) override;

};

}

#endif
