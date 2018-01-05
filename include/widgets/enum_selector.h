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
#ifndef SOLARUSEDITOR_ENUM_SELECTOR_H
#define SOLARUSEDITOR_ENUM_SELECTOR_H

#include <QComboBox>

namespace SolarusEditor {

/**
 * @brief Combobox to select a value from an enumerated type.
 *
 * The combobox is automatically filled with appropriate icons
 * and translated names for type E as provided by EnumTraits<E>.
 * Values of the enumerated type E must be indexed from 0 to n-1
 * where n is the number of values.
 */
template<typename E>
class EnumSelector : public QComboBox {

public:

  EnumSelector(QWidget* parent = nullptr);

  bool is_with_none() const;
  void set_with_none(bool with_none);

  bool is_empty() const;
  void set_empty();

  E get_selected_value() const;
  void set_selected_value(const E& value);

private:

  bool with_none;                 /**< Whether an empty option is included in the list. */

};

}

#include "widgets/enum_selector.inl"

#endif
