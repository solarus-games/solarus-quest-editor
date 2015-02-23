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
#include "gui/mouse_coordinates_tracking_tool.h"
#include <QAbstractScrollArea>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>
#include <QScrollBar>

/**
 * @brief Creates a tool that tracks the specified widget.
 * @param widget The widget to track.
 */
MouseCoordinatesTrackingTool::MouseCoordinatesTrackingTool(QAbstractScrollArea* widget) :
  QObject(widget) {

  widget->viewport()->installEventFilter(this);
}

/**
 * @brief Filters mouse move events.
 * @param object The watched object.
 * @param event An incoming event.
 * @return @c true to stop the propagation of the event.
 */
bool MouseCoordinatesTrackingTool::eventFilter(QObject* object, QEvent* event) {

  QAbstractScrollArea* scroll_area = qobject_cast<QAbstractScrollArea*>(parent());

  if (scroll_area == nullptr || object != scroll_area->viewport()) {
    return QObject::eventFilter(object, event);
  }

  if (event->type() == QEvent::MouseMove) {
    QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
    const char* method = "mouse_coordinates_changed";
    QMetaObject::invokeMethod(
          scroll_area,
          method,
          Qt::DirectConnection,
          Q_ARG(QPoint, mouse_event->pos()));
    // Don't stop the event.
  }

  else if (event->type() == QEvent::Leave) {
    const char* method = "mouse_left";
    QMetaObject::invokeMethod(
          scroll_area,
          method,
          Qt::DirectConnection);
    // Don't stop the event.
  }

  return QObject::eventFilter(object, event);
}
