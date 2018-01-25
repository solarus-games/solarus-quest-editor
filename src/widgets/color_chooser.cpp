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
#include "widgets/color_chooser.h"
#include <QColorDialog>

namespace SolarusEditor {

namespace {

const QString style_sheet =
    "SolarusEditor--ColorChooser {\n"
    "    background-color: %1;\n"
    "    border-style: inset;\n"
    "    border-width: 2px;\n"
    "    border-color: gray;\n"
    "    min-width: 1em;\n"
    "    padding: 1px;\n"
    "}";

}  // Anonymous namespace.

/**
 * @brief Creates a color picker.
 * @param parent The parent widget.
 */
ColorChooser::ColorChooser(QWidget *parent) :
  QPushButton(parent),
  color(Qt::white) {

  update_style_sheet();

  connect(this, SIGNAL(clicked()), this, SLOT(pick_color_requested()));
}

/**
 * @brief Returns the current color of the picker.
 * @return The current color.
 */
QColor ColorChooser::get_color() const {

  return color;
}

/**
 * @brief Sets the color of the picker.
 *
 * Emits color_changed() if there is a change.
 *
 * @param color The new color.
 */
void ColorChooser::set_color(const QColor& color) {

  if (color == this->color) {
    return;
  }

  this->color = color;
  update_style_sheet();
  emit color_changed(color);
}

/**
 * @brief Updates the stylesheet of the chooser.
 */
void ColorChooser::update_style_sheet() {

  setStyleSheet(style_sheet.arg(color.name()));
}

/**
 * @brief Slot called when the user wants to pick another color.
 */
void ColorChooser::pick_color_requested() {

  QColor new_color = QColorDialog::getColor(color, this, tr("Select color"));

  if (new_color.isValid()) {
    set_color(new_color);
  }
}

}
