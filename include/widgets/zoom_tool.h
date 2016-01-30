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
#ifndef SOLARUSEDITOR_ZOOM_TOOL_H
#define SOLARUSEDITOR_ZOOM_TOOL_H

#include <QObject>

class QAbstractScrollArea;

/**
 * @brief Provides to a scrolling widget the ability to zoom the view using
 * the middle mouse wheel and the control key.
 *
 * The controlled scrolling widget must have slots zoom_in() and zoom_out().
 */
class ZoomTool : public QObject {
  Q_OBJECT

public:

  ZoomTool(QAbstractScrollArea* widget);

  bool eventFilter(QObject* object, QEvent* event) override;

};

#endif
