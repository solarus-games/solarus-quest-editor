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
#ifndef SOLARUSEDITOR_PLAIN_TEXT_EDIT_H
#define SOLARUSEDITOR_PLAIN_TEXT_EDIT_H

#include <QPlainTextEdit>

/**
 * @brief A plain text editor that sends editing_finished() signal when the text
 * was changed and focus is lost.
 */
class PlainTextEdit : public QPlainTextEdit {
  Q_OBJECT

public:

  inline PlainTextEdit(QWidget* parent = nullptr) :
    QPlainTextEdit(parent),
    changed(false) {
    setTabChangesFocus(true);
    connect(this, SIGNAL(textChanged()), this, SLOT(handle_text_changed()));
  }

signals:

  void editing_finished();

protected:

    inline virtual void focusOutEvent(QFocusEvent *event) override {
      if (changed) {
        emit editing_finished();
        changed = false;
      }
      QPlainTextEdit::focusOutEvent(event);
    }

private slots:

  inline void handle_text_changed() {
    changed = true;
  }

private:

  bool changed;

};

#endif
