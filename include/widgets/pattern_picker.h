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
#ifndef SOLARUSEDITOR_PATTERN_PICKER_H
#define SOLARUSEDITOR_PATTERN_PICKER_H

#include <QPushButton>

namespace SolarusEditor {

/**
 * @brief Widget to pick a pattern in a tileset.
 */
class PatternPicker : public QPushButton {
  Q_OBJECT

public:

  explicit PatternPicker(QWidget* parent = nullptr);

  QString get_pattern_id() const;
  void set_pattern_id(const QString& pattern_id);

public slots:

  void pick_pattern_requested();

signals:

  void pattern_id_changed(const QString& pattern_id);

private:

  QString pattern_id;

};

}

#endif
