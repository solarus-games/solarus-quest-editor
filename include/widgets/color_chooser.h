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
#ifndef SOLARUSEDITOR_COLOR_CHOOSER_H
#define SOLARUSEDITOR_COLOR_CHOOSER_H

#include <QPushButton>

namespace SolarusEditor {

/**
 * @brief Widget to choose a color.
 */
class ColorChooser : public QPushButton {
  Q_OBJECT

public:

  explicit ColorChooser(QWidget* parent = nullptr);

  QColor get_color() const;
  void set_color(const QColor& color);

public slots:

  void pick_color_requested();

signals:

  void color_changed(const QColor& color);

private:

  void update_style_sheet();

  QColor color;

};

}

#endif
