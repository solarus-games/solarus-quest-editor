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
#ifndef SOLARUSEDITOR_PAIR_SPIN_BOX_H
#define SOLARUSEDITOR_PAIR_SPIN_BOX_H

#include <QWidget>
#include <QSpinBox>
#include <QLabel>

namespace SolarusEditor {

/**
 * @brief Widget with two spinboxes to edit a pair of integer.
 */
class PairSpinBox : public QWidget {
  Q_OBJECT

public:

  PairSpinBox(QWidget* parent = nullptr);

  int get_first_value() const;
  int get_second_value() const;

  int get_first_max() const;
  int get_second_max() const;

  int get_first_min() const;
  int get_second_min() const;

  int get_first_step() const;
  int get_second_step() const;

  QPair<int, int> get_values() const;
  QPoint get_point() const;
  QSize get_size() const;

  QString get_separator_text() const;

  bool get_first_enabled() const;
  bool get_second_enabled() const;

public slots:

  void set_first_value(int value);
  void set_second_value(int value);

  void set_first_max(int max);
  void set_second_max(int max);

  void set_first_min(int min);
  void set_second_min(int min);

  void set_first_step(int step);
  void set_second_step(int step);

  void set_first_tooltip(const QString& text);
  void set_second_tooltip(const QString& text);

  void set_values(int first_value, int second_value);
  void set_values(const QPair<int, int>& values);
  void set_point(const QPoint& values);
  void set_size(const QSize& values);

  void set_max(int max);
  void set_min(int min);
  void set_step(int step);

  void set_tooltips(const QString& first_text, const QString& second_text);

  void config(
    const QString& separator_text, int min = 0, int max = 99999, int step = 1);

  void set_first_enabled(bool enabled);
  void set_second_enabled(bool enabled);

signals:

  void first_value_changed(int first_value);
  void second_value_changed(int second_value);
  void value_changed(int first_value, int second_value);
  void editing_finished();

private slots:

  void on_first_value_changed();
  void on_second_value_changed();

private:

  QSpinBox first_spin_box;
  QSpinBox second_spin_box;
  QLabel separator_label;

};

}

#endif
