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
#include "widgets/zoom_tool.h"
#include <QAbstractScrollArea>
#include <QApplication>
#include <QWheelEvent>

namespace SolarusEditor {

/**
 * @brief Creates a zoom tool on the specified object.
 * @param widget The scrolling area to manage.
 */
ZoomTool::ZoomTool(QAbstractScrollArea* widget) :
  QObject(widget) {

  widget->viewport()->installEventFilter(this);
}

/**
 * @brief Filters zoom events.
 * @param object The watched object.
 * @param event An incoming event.
 * @return @c true to stop the propagation of the event.
 */
bool ZoomTool::eventFilter(QObject* object, QEvent* event) {

  QAbstractScrollArea* scroll_area = qobject_cast<QAbstractScrollArea*>(parent());

  if (scroll_area == nullptr || object != scroll_area->viewport()) {
    return QObject::eventFilter(object, event);
  }

  if (event->type() == QEvent::Wheel) {
    QWheelEvent* wheel_event = static_cast<QWheelEvent*>(event);

    if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
      // Control + wheel: zoom in or out.
      const char* method = wheel_event->delta() > 0 ? "zoom_in" : "zoom_out";
      QMetaObject::invokeMethod(
            scroll_area,
            method,
            Qt::DirectConnection);
      return true;
    }
  }

  return QObject::eventFilter(object, event);
}

}
