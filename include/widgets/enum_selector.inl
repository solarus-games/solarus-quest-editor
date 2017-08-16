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
namespace SolarusEditor {

template<typename E>
class EnumTraits;

/**
 * @brief Creates an enum selector.
 * @param parent The parent widget or nullptr.
 */
template<typename E>
EnumSelector<E>::EnumSelector(QWidget* parent) :
  QComboBox(parent),
  with_none(false) {

  Q_FOREACH (const E& value, EnumTraits<E>::get_values()) {
    addItem(EnumTraits<E>::get_icon(value),
            EnumTraits<E>::get_friendly_name(value));
  }
}

/**
 * @brief Returns whether this selector includes an empty option.
 * @return @c true if an empty option is included.
 */
template<typename E>
bool EnumSelector<E>::is_with_none() const {
  return with_none;
}

/**
 * @brief Sets whether this selector includes an empty option.
 * @param with_none @c true to include an empty option.
 */
template<typename E>
void EnumSelector<E>::set_with_none(bool with_none) {

  if (with_none == this->with_none) {
    return;
  }

  this->with_none = with_none;
  if (with_none) {
    insertItem(0, "");
  }
  else {
    removeItem(0);
  }
}

/**
 * @brief Returns whether no value is selected.
 *
 * Only possible if the selector includes an empty option.
 *
 * @return @c true if no value is selected.
 */
template<typename E>
bool EnumSelector<E>::is_empty() const {

  return with_none && currentIndex() == 0;
}

/**
 * @brief Unselects any value.
 *
 * Only possible if the selector includes an empty option
 * (otherwise this function has no effect).
 */
template<typename E>
void EnumSelector<E>::set_empty() {

  if (!is_with_none()) {
    return;
  }

  setCurrentIndex(0);
}

/**
 * @brief Returns the selected value.
 *
 * It is an error to call this function is no value is selected
 * (in this case a default-constructed value is returned).
 *
 * @return The selected value.
 */
template<typename E>
E EnumSelector<E>::get_selected_value() const {

  if (is_empty()) {
    return E();
  }

  int index = currentIndex();
  if (with_none) {
    --index;
  }
  return static_cast<E>(index);
}

/**
 * @brief Changes the selected value.
 * @param value The value to select.
 */
template<typename E>
void EnumSelector<E>::set_selected_value(const E& value) {

  int index = static_cast<int>(value);
  if (with_none) {
    ++index;
  }
  setCurrentIndex(index);
}

}
