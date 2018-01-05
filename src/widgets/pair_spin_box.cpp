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
#include "widgets/pair_spin_box.h"
#include <QHBoxLayout>

namespace SolarusEditor {

/**
 * @brief Creates a pair spin box.
 * @param parent The parent widget.
 */
PairSpinBox::PairSpinBox(QWidget *parent) :
  QWidget(parent) {

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setMargin(0);

  layout->addWidget(&first_spin_box);
  layout->addWidget(&separator_label);
  layout->addWidget(&second_spin_box);
  layout->addSpacerItem(new QSpacerItem(0, 22, QSizePolicy::Expanding));

  connect(&first_spin_box, SIGNAL(valueChanged(int)),
          this, SLOT(on_first_value_changed()));
  connect(&second_spin_box, SIGNAL(valueChanged(int)),
          this, SLOT(on_second_value_changed()));

  connect(&first_spin_box, SIGNAL(editingFinished()),
          this, SIGNAL(editing_finished()));
  connect(&second_spin_box, SIGNAL(editingFinished()),
          this, SIGNAL(editing_finished()));
}

/**
 * @brief Returns the first value.
 * @return The first value.
 */
int PairSpinBox::get_first_value() const {

  return first_spin_box.value();
}

/**
 * @brief Returns the second value.
 * @return The second value.
 */
int PairSpinBox::get_second_value() const {

  return second_spin_box.value();
}

/**
 * @brief Returns the first maximum value.
 * @return The first maximum value.
 */
int PairSpinBox::get_first_max() const {

  return first_spin_box.maximum();
}

/**
 * @brief Returns the second maximum value.
 * @return The second maximum value.
 */
int PairSpinBox::get_second_max() const {

  return second_spin_box.maximum();
}

/**
 * @brief Returns the first minimum value.
 * @return The first minimum value.
 */
int PairSpinBox::get_first_min() const {

  return first_spin_box.minimum();
}

/**
 * @brief Returns the second minimum value.
 * @return The second minimum value.
 */
int PairSpinBox::get_second_min() const {

  return second_spin_box.minimum();
}

/**
 * @brief Returns the first step value.
 * @return The first step value.
 */
int PairSpinBox::get_first_step() const {

  return first_spin_box.singleStep();
}

/**
 * @brief Returns the second step value.
 * @return The second step value.
 */
int PairSpinBox::get_second_step() const {

  return second_spin_box.singleStep();
}

/**
 * @brief Returns the values.
 * @return The values.
 */
QPair<int, int> PairSpinBox::get_values() const {

  return QPair<int, int>(get_first_value(), get_second_value());
}

/**
 * @brief Returns the point value.
 * @return The point value.
 */
QPoint PairSpinBox::get_point() const {

  return QPoint(get_first_value(), get_second_value());
}

/**
 * @brief Returns the size value.
 * @return The size value.
 */
QSize PairSpinBox::get_size() const {

  return QSize(get_first_value(), get_second_value());
}

/**
 * @brief Returns the separator text.
 * @return The separator text.
 */
QString PairSpinBox::get_separator_text() const {

  return separator_label.text();
}

/**
 * @brief Returns whether the first spin box is enabled.
 * @return @c true if the first spin box is enabled.
 */
bool PairSpinBox::get_first_enabled() const {

  return first_spin_box.isEnabled();
}

/**
 * @brief Returns whether the second spin box is enabled.
 * @return @c true if the second spin box is enabled.
 */
bool PairSpinBox::get_second_enabled() const {

  return second_spin_box.isEnabled();
}

/**
 * @brief Changes the first value.
 * @param value The new value.
 */
void PairSpinBox::set_first_value(int value) {

  first_spin_box.setValue(value);
}

/**
 * @brief Changes the second value.
 * @param value The new value.
 */
void PairSpinBox::set_second_value(int value) {

  second_spin_box.setValue(value);
}

/**
 * @brief Changes the first maximum value.
 * @param max The new maximum value.
 */
void PairSpinBox::set_first_max(int max) {

  first_spin_box.setMaximum(max);
}

/**
 * @brief Changes the second maximum value.
 * @param max The new maximum value.
 */
void PairSpinBox::set_second_max(int max) {

  second_spin_box.setMaximum(max);
}

/**
 * @brief Changes the first minimum value.
 * @param min The new minimum value.
 */
void PairSpinBox::set_first_min(int min) {

  first_spin_box.setMinimum(min);
}

/**
 * @brief Changes the second minimum value.
 * @param min The new minimum value.
 */
void PairSpinBox::set_second_min(int min) {

  second_spin_box.setMinimum(min);
}

/**
 * @brief Changes the first step value.
 * @param step The new step value.
 */
void PairSpinBox::set_first_step(int step) {

  first_spin_box.setSingleStep(step);
}

/**
 * @brief Changes the second step value.
 * @param step The new step value.
 */
void PairSpinBox::set_second_step(int step) {

  second_spin_box.setSingleStep(step);
}

/**
 * @brief Changes the first tooltip.
 * @param text The new tooltip.
 */
void PairSpinBox::set_first_tooltip(const QString &text) {

  first_spin_box.setToolTip(text);
}

/**
 * @brief Changes the second tooltip.
 * @param text The new tooltip.
 */
void PairSpinBox::set_second_tooltip(const QString &text) {

  second_spin_box.setToolTip(text);
}

/**
 * @brief Changes values.
 * @param first_value The new first value.
 * @param second_value The new second value.
 */
void PairSpinBox::set_values(int first_value, int second_value) {

  set_first_value(first_value);
  set_second_value(second_value);
}

/**
 * @brief Changes values.
 * @param values The new values.
 */
void PairSpinBox::set_values(const QPair<int, int> &values) {

  set_values(values.first, values.second);
}

/**
 * @brief Changes values.
 * @param values The new values.
 */
void PairSpinBox::set_point(const QPoint& values) {

  set_first_value(values.x());
  set_second_value(values.y());
}

/**
 * @brief Changes values.
 * @param values The new values.
 */
void PairSpinBox::set_size(const QSize& values) {

  set_first_value(values.width());
  set_second_value(values.height());
}

/**
 * @brief Changes maximum values.
 * @param max The new maximum values.
 */
void PairSpinBox::set_max(int max) {

  set_first_max(max);
  set_second_max(max);
}

/**
 * @brief Changes minimum values.
 * @param min The new minimum values.
 */
void PairSpinBox::set_min(int min) {

  set_first_min(min);
  set_second_min(min);
}

/**
 * @brief Changes step values.
 * @param step The new step values.
 */
void PairSpinBox::set_step(int step) {

  set_first_step(step);
  set_second_step(step);
}

/**
 * @brief Changes tooltips.
 * @param first_text The new first tooltip.
 * @param second_text The new second tooltip.
 */
void PairSpinBox::set_tooltips(
  const QString &first_text, const QString &second_text) {

  set_first_tooltip(first_text);
  set_second_tooltip(second_text);
}

/**
 * @brief Configures the widget.
 * @param separator_text The new separator text.
 * @param min The new minimum values.
 * @param max The new maximum values.
 * @param step The new step values.
 */
void PairSpinBox::config(
  const QString &separator_text, int min, int max, int step) {

  separator_label.setText(separator_text);
  set_min(min);
  set_max(max);
  set_step(step);
}

/**
 * @brief Changes the enable state of the first spin box.
 * @param enabled @c true to enable the spin box.
 */
void PairSpinBox::set_first_enabled(bool enabled) {

  first_spin_box.setEnabled(enabled);
}

/**
 * @brief Changes the enable state of the second spin box.
 * @param enabled @c true to enable the spin box.
 */
void PairSpinBox::set_second_enabled(bool enabled) {

  second_spin_box.setEnabled(enabled);
}

/**
 * @brief Slot called when the user has just change the first value.
 */
void PairSpinBox::on_first_value_changed() {

  emit first_value_changed(get_first_value());
  emit value_changed(get_first_value(), get_second_value());
}

/**
 * @brief Slot called when the user has just change the second value.
 */
void PairSpinBox::on_second_value_changed() {

  emit second_value_changed(get_second_value());
  emit value_changed(get_first_value(), get_second_value());
}

}
