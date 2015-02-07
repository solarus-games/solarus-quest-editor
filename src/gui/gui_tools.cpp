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
#include "gui/gui_tools.h"
#include <QMessageBox>
#include <QPainter>

namespace GuiTools {

/**
 * @brief Shows a modal dialog box with an information message.
 * @param message The message to show.
 */
void information_dialog(const QString& message) {

  QMessageBox messageBox;
  messageBox.setIcon(QMessageBox::Information);
  messageBox.setText(message);
  messageBox.setWindowTitle(QMessageBox::tr("Information"));
  messageBox.exec();
}

/**
 * @brief Shows a modal dialog box with a warning message.
 * @param message The message to show.
 */
void warning_dialog(const QString& message) {

  QMessageBox messageBox;
  messageBox.setIcon(QMessageBox::Warning);
  messageBox.setText(message);
  messageBox.setWindowTitle(QMessageBox::tr("Warning"));
  messageBox.exec();
}

/**
 * @brief Shows a modal dialog box with an error message.
 * @param message The message to show.
 */
void error_dialog(const QString& message) {

  QMessageBox messageBox;
  messageBox.setIcon(QMessageBox::Critical);
  messageBox.setText(message);
  messageBox.setWindowTitle(QMessageBox::tr("Error"));
  messageBox.exec();
}

/**
 * @brief Draws a rectangle outline.
 *
 * Unlike QPainter::drawRect(), this function draws the outline entirely
 * inside the rectangle and does not involve half-pixels.
 *
 * @param painter The painter.
 * @param where Rectangle to draw the outline of.
 * @param color Color to use.
 * @param thickness Thickness of the brush.
 */
void draw_rectangle_outline(QPainter& painter,
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
 * @brief Draws a grid.
 * @param painter The painter to draw.
 * @param where Rectangle where drawing the grid should be limited to.
 * @param spacing Square size.
 */
void draw_grid(QPainter& painter, const QRect& where, int square_size) {

  int left = where.left() - where.left() % square_size;
  int top = where.top() - where.top() % square_size;

  QVarLengthArray<QLineF, 100> lines;

  for (int x = left; x < where.right(); x += square_size) {
    lines.append(QLineF(x, where.top(), x, where.bottom()));
  }

  for (int y = top; y < where.bottom(); y += square_size) {
    lines.append(QLineF(where.left(), y, where.right(), y));
  }

  painter.setPen(QPen(Qt::black, 0, Qt::DashLine));
  painter.drawLines(lines.data(), lines.size());

}

}
