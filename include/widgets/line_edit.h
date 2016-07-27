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
#ifndef SOLARUSEDITOR_LINE_EDIT_H
#define SOLARUSEDITOR_LINE_EDIT_H

#include <QLineEdit>

namespace SolarusEditor {

/**
 * @brief A line editor that sends focus_in() and focus_lost() signals.
 */
class LineEdit : public QLineEdit {
  Q_OBJECT

public:

  inline LineEdit(QWidget* parent = nullptr) :
    QLineEdit(parent) {
  }

signals:

  void focus_in(QFocusEvent* event);
  void focus_out(QFocusEvent* event);

protected:

  inline virtual void focusInEvent(QFocusEvent* event) override {
    emit focus_in(event);
    QLineEdit::focusInEvent(event);
  }

  inline virtual void focusOutEvent(QFocusEvent* event) override {
    emit focus_out(event);
    QLineEdit::focusOutEvent(event);
  }

};

}

#endif
