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
#include "gui/pan_tool.h"
#include <QAbstractScrollArea>
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QScrollBar>

/**
 * @brief Creates a pan tool on the specified object.
 * @param widget The scrolling area to manage.
 */
PanTool::PanTool(QAbstractScrollArea* widget) :
  QObject(widget) {

  widget->viewport()->installEventFilter(this);
}

/**
 * @brief Filters panning events.
 * @param object The watched object.
 * @param event An incoming event.
 * @return @c true to stop the propagation of the event.
 */
bool PanTool::eventFilter(QObject* object, QEvent* event) {

  QAbstractScrollArea* scroll_area = qobject_cast<QAbstractScrollArea*>(parent());

  if (scroll_area == nullptr || object != scroll_area->viewport()) {
    return QObject::eventFilter(object, event);
  }

  if (event->type() == QEvent::MouseButtonPress) {
    QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);

    if (mouse_event->button() == Qt::MidButton) {
      // Middle button pressed: start panning the view.
      QApplication::setOverrideCursor(Qt::ClosedHandCursor);
      pan_initial_point = QPoint(
            scroll_area->horizontalScrollBar()->value() + mouse_event->x(),
            scroll_area->verticalScrollBar()->value() + mouse_event->y()
            );

      return true;
    }
  }

  else if (event->type() == QEvent::MouseMove) {
    QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);

    if ((mouse_event->buttons() & Qt::MidButton) == Qt::MidButton) {
      // Moved the mouse while pressing middle button: pan the view.
      QPoint scroll_point(
            pan_initial_point.x() - mouse_event->x(),
            pan_initial_point.y() - mouse_event->y()
            );
      scroll_area->horizontalScrollBar()->setValue(scroll_point.x());
      scroll_area->verticalScrollBar()->setValue(scroll_point.y());

      return true;
    }
  }

  else if (event->type() == QEvent::MouseButtonRelease) {
    QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);

    if (mouse_event->button() == Qt::MidButton) {
      QApplication::restoreOverrideCursor();
      return true;
    }
  }

  return QObject::eventFilter(object, event);
}
