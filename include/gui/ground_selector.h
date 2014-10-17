/*
 * Copyright (C) 2014 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_GROUND_SELECTOR_H
#define SOLARUSEDITOR_GROUND_SELECTOR_H

#include <solarus/entities/Ground.h>
#include <QComboBox>

using Ground = Solarus::Ground;

/**
 * @brief A combobox to select a Solarus ground value.
 *
 * Each ground value is displayed with a user-friendly name and an icon.
 */
class GroundSelector : public QComboBox {

public:

  GroundSelector(QWidget* parent = nullptr);

  bool is_with_none() const;
  void set_with_none(bool with_none);

  bool is_empty() const;
  void set_empty();

  Ground get_selected_ground() const;
  void set_selected_ground(Ground ground);

  QString get_ground_friendly_name(Ground ground) const;

private:

  void initialize_translated_names();

  QMap<Ground, QString>
      ground_friendly_names;      /**< Human-readable name of each ground value. */
  bool with_none;                 /**< Whether an empty option is included in the list. */

};

#endif
