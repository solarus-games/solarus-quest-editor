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
#include "widgets/pattern_picker.h"
#include <QInputDialog>

namespace SolarusEditor {

/**
 * @brief Creates a pattern picker.
 * @param parent The parent widget.
 */
PatternPicker::PatternPicker(QWidget *parent) :
  QPushButton(parent),
  pattern_id() {

  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setIconSize(QSize(32, 32));
  setIcon(QIcon(":/images/entity_tile.png"));

  connect(this, SIGNAL(clicked()), this, SLOT(pick_pattern_requested()));
}

/**
 * @brief Returns the current pattern of the picker.
 * @return Id of the current pattern.
 */
QString PatternPicker::get_pattern_id() const {

  return pattern_id;
}

/**
 * @brief Sets the pattern of the picker.
 *
 * Emits pattern_id_changed() if there is a change.
 *
 * @param pattern_id The pattern id to set.
 */
void PatternPicker::set_pattern_id(const QString& pattern_id) {

  if (pattern_id == this->pattern_id) {
    return;
  }

  this->pattern_id = pattern_id;
  setText(pattern_id);
  // TODO set the button icon
  emit pattern_id_changed(pattern_id);
}

/**
 * @brief Slot called when the user want to pick another pattern.
 */
void PatternPicker::pick_pattern_requested() {

  // TODO open a dialog with a tileset view instead
  bool ok = false;
  QString pattern_id = QInputDialog::getText(
        this,
        tr("Pattern"),
        tr("Pattern id:"),
        QLineEdit::Normal,
        "",
        &ok);

  if (ok && !pattern_id.isEmpty()) {
    set_pattern_id(pattern_id);
  }
}

}
