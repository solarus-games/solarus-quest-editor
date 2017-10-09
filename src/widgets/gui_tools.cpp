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
#include "widgets/gui_tools.h"
#include <solarus/gui/gui_tools.h>
#include <QMessageBox>
#include <QPainter>

namespace SolarusEditor {

namespace GuiTools {

/**
 * @brief Shows a modal dialog box with an information message.
 * @param message The message to show.
 */
void information_dialog(const QString& message) {

  SolarusGui::GuiTools::information_dialog(message);
}

/**
 * @brief Shows a modal dialog box with a warning message.
 * @param message The message to show.
 */
void warning_dialog(const QString& message) {

  SolarusGui::GuiTools::warning_dialog(message);
}

/**
 * @brief Shows a modal dialog box with an error message.
 * @param message The message to show.
 */
void error_dialog(const QString& message) {

  SolarusGui::GuiTools::error_dialog(message);
}

/**
 * @brief Draws a rectangle border.
 *
 * Unlike QPainter::drawRect(), this function draws the outline entirely
 * inside the rectangle and does not involve half-pixels.
 *
 * @param painter The painter.
 * @param where Rectangle to draw the outline of.
 * @param color Color to use.
 * @param thickness Thickness of the brush.
 */
void draw_rectangle_border(QPainter& painter,
                           const QRect& where,
                           const QColor& color,
                           int thickness) {
  const int x = where.x();
  const int y = where.y();
  const int w = where.width();
  const int h = where.height();
  const int t = thickness;
  QBrush brush(color);
  painter.fillRect(QRect(        x,         y, w, t), brush);
  painter.fillRect(QRect(        x, y + h - t, w, t), brush);
  painter.fillRect(QRect(        x,         y, t, h), brush);
  painter.fillRect(QRect(x + w - t,         y, t, h), brush);

}

/**
 * @brief Draws a rectangle border as two black lines and a color between them.
 *
 * The black lines will always keep a thickness of one pixel
 * no matter the transformation set on the painter.
 *
 * @param painter The painter.
 * @param where Rectangle to draw the outline of.
 * @param color Color to use.
 * @param thickness Thickness of the brush.
 */
void draw_rectangle_border_double(QPainter& painter,
                                  const QRect& where,
                                  const QColor& color_between) {

  draw_rectangle_border(painter, where, color_between, 2);

  painter.setPen(QPen(Qt::black, 0, Qt::SolidLine));
  QRect dst = where;
  painter.drawRect(dst);
  dst.adjust(2, 2, -2, -2);
  painter.drawRect(dst);
}

/**
 * @brief Draws a grid with dashed lines.
 *
 * The lines of the grid will always keep a thickness of one pixel
 * no matter the transformation set on the painter.
 *
 * @param painter The painter to draw.
 * @param where Rectangle where drawing the grid should be limited to.
 * @param size Grid size.
 * @param color Grid color.
 * @param style Grid style.
 */
void draw_grid(QPainter& painter, const QRect& where,
  const QSize &size, const QColor& color, GridStyle style) {

  if (style == GridStyle::INTERSECT_POINT) {
    draw_grid_point(painter, where, size, color);
    return;
  }

  QVarLengthArray<QLineF, 100> lines;

  if (style == GridStyle::INTERSECT_CROSS) {

    for (int x = where.left(); x < where.right(); x += size.width()) {
      for (int y = where.top(); y < where.bottom(); y += size.height()) {
        lines.append(QLineF(x - 2, y, x + 2, y));
        lines.append(QLineF(x, y - 2, x, y + 2));
      }
    }

  } else {

    for (int x = where.left(); x < where.right(); x += size.width()) {
      lines.append(QLineF(x, where.top(), x, where.bottom()));
    }
    for (int y = where.top(); y < where.bottom(); y += size.height()) {
      lines.append(QLineF(where.left(), y, where.right(), y));
    }
  }

  QPen pen(color, 0);
  if (style == GridStyle::DASHED) {
    pen.setStyle(Qt::DashLine);
  }

  painter.setPen(pen);
  painter.drawLines(lines.data(), lines.size());

}

/**
 * @brief Draws a grid with points.
 * @param painter The painter to draw.
 * @param where Rectangle where drawing the grid should be limited to.
 * @param size Grid size.
 * @param color Grid color.
 */
void draw_grid_point(
  QPainter& painter, const QRect& where,
  const QSize &size, const QColor& color) {

  QVarLengthArray<QPointF, 100> points;

  for (int x = where.left(); x < where.right(); x += size.width()) {
    for (int y = where.top(); y < where.bottom(); y += size.height()) {
      points.append(QPointF(x, y));
    }
  }

  painter.setPen(QPen(color, 1));
  painter.drawPoints(points.data(), points.size());
}

}

}
