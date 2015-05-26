/*
 * Copyright (C) 2014-2015 Christopho, Solarus - http://www.solarus-games.org
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
#include <QTimer>

/**
 * @brief A line editor.
 *
 * This widget use a QTimer to send the signal editing_finished() after 3/4
 * of seconds when the user stops to input text and if the value has changed.
 * The signal editing_finished() is also emitted when the focus is lost
 * and the value changed.
 */
class LineEdit : public QLineEdit {
  Q_OBJECT

public:

  inline LineEdit(QWidget* parent = nullptr) :
    QLineEdit(parent),
    timer(this) {

    timer.setInterval(750);
    timer.setSingleShot(true);

    connect(this, SIGNAL(textEdited(QString)), this, SLOT(on_text_edited()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_timeout()));
  }

signals:

  void editing_finished();

protected:

  inline void focusInEvent(QFocusEvent* event) override {
    before = text();
    QLineEdit::focusInEvent(event);
  }

  inline void focusOutEvent(QFocusEvent* event) override {

    if (timer.isActive()) {
      timer.stop();
    }
    on_timeout();
    QLineEdit::focusOutEvent(event);
  }

private slots:

  inline void on_text_edited() {

    if (timer.isActive()) {
      timer.stop();
    }
    if (text() != before) {
      timer.start();
    }
  }

  inline void on_timeout() {

    if (text() != before) {
      before = text();
      emit editing_finished();
    }
  }

private:

  QString before;             /**< The value before the current editing. */
  QTimer timer;               /**< The timer. */

};

#endif
