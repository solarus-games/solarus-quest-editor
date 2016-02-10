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
#include "widgets/color_picker.h"
#include <QColorDialog>

namespace SolarusEditor {

const QString ColorPicker::style_sheet =
  ".ColorPicker {\n"
  "    background-color: %1;\n"
  "    border-style: inset;\n"
  "    border-width: 2px;\n"
  "    border-color: gray;\n"
  "    min-width: 1em;\n"
  "    padding: 1px;\n"
  "}";

/**
 * @brief Creates a color picker.
 * @param parent The parent widget.
 */
ColorPicker::ColorPicker(QWidget *parent) :
  QPushButton(parent),
  color(Qt::white) {

  update_style_sheet();

  connect(this, SIGNAL(clicked()), this, SLOT(change_color()));
}

/**
 * @brief Returns the current color of the picker.
 * @return The current color.
 */
QColor ColorPicker::get_color() const {

  return color;
}

/**
 * @brief Changes the color of the picker.
 *
 * Emits color_changed() if the color has changed.
 *
 * @param color The new color.
 */
void ColorPicker::set_color(const QColor& color) {

  if (color == this->color) {
    return;
  }

  this->color = color;
  update_style_sheet();
  emit color_changed(color);
}

/**
 * @brief Slot called when the user want to change the color.
 */
void ColorPicker::change_color() {

  QColor new_color = QColorDialog::getColor(color, this, tr("Select color"));

  if (new_color.isValid()) {
    set_color(new_color);
  }
}

/**
 * @brief Update the stylesheet of the color picker.
 */
void ColorPicker::update_style_sheet() {

  setStyleSheet(style_sheet.arg(color.name()));
}

}
