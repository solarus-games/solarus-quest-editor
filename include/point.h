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
#ifndef SOLARUSEDITOR_POINT_H
#define SOLARUSEDITOR_POINT_H

#include <solarus/lowlevel/Point.h>
#include <QPoint>

class QSize;

namespace SolarusEditor {

namespace Point {

QPoint to_qpoint(const Solarus::Point& solarus_point);
Solarus::Point to_solarus_point(const QPoint& qt_point);

QPoint round_8(const QPoint& point);
QPoint round_8(const QPointF& point);

QPoint floor_8(const QPoint& point);
QPoint floor_8(const QPointF& point);

QPoint floor(const QPoint& point, const QSize& size);
QPoint floor(const QPointF& point, const QSize& size);

QPoint ceil(const QPoint& point, const QSize& size);
QPoint ceil(const QPointF& point, const QSize& size);

QPoint round_down(const QPoint& point, const QSize& size);
QPoint round_down(const QPointF& point, const QSize& size);

}

}

#endif
