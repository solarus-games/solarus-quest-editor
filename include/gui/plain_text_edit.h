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
#include <QPainter>
#include <QTimer>

/**
 * @brief A plain text editor that sends editing_finished() signal when the text
 * was changed and focus is lost.
 * This editor provides also a method to display a margin line at a specific
 * column.
 */
class PlainTextEdit : public QPlainTextEdit {
  Q_OBJECT

public:

  inline PlainTextEdit(QWidget* parent = nullptr) :
    QPlainTextEdit(parent),
    show_margin(false),
    timer(this) {

    setTabChangesFocus(true);
    setLineWrapMode(LineWrapMode::NoWrap);

    QTextCharFormat char_format = currentCharFormat();
    char_format.setFontFixedPitch(true);
    setCurrentCharFormat(char_format);

    timer.setInterval(750);
    timer.setSingleShot(true);

    connect(this, SIGNAL(textChanged()), this, SLOT(handle_text_changed()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_timeout()));
  }

  /**
   * @brief Like setPlainText() method but keep the cursor position.
   * @param text The new text.
   */
  inline void set_plain_text(const QString& text) {

    int cursor_position = textCursor().position();
    setPlainText(text);
    QTextCursor cursor = textCursor();
    cursor.setPosition(cursor_position);
    setTextCursor(cursor);
  }

  inline void set_show_margin(bool show_margin, int margin = 0) {

    this->show_margin = show_margin && margin > 0;
    this->margin = margin;
    viewport()->repaint();
  }

signals:

  void editing_finished();

protected:

  inline void focusInEvent(QFocusEvent* event) override {

    before = toPlainText();
    QPlainTextEdit::focusOutEvent(event);
  }

  inline void focusOutEvent(QFocusEvent* event) override {

    if (timer.isActive()) {
      timer.stop();
    }
    on_timeout();
    QPlainTextEdit::focusOutEvent(event);
  }

  inline void paintEvent(QPaintEvent* event) override {

    if (show_margin) {
      const QRect rect = event->rect();
      const QFont font = currentCharFormat().font();
      int x = round(QFontMetrics(font).maxWidth() * margin)
            + contentOffset().x()
            + document()->documentMargin();

      QPainter p(viewport());
      p.setPen(QPen(isEnabled() ? "blue" : "gray"));
      p.drawLine(x, rect.top(), x, rect.bottom());
    }

    QPlainTextEdit::paintEvent(event);
  }

private slots:

  inline void handle_text_changed() {

    if (timer.isActive()) {
      timer.stop();
    }
    if (toPlainText() != before) {
      timer.start();
    }
  }

  inline void on_timeout() {

    QString text = toPlainText();
    if (text != before) {
      before = text;
      emit editing_finished();
    }
  }

private:

  QString before;     /**< The value before the current editing. */
  bool show_margin;   /**< To show a margin. */
  int margin;         /**< The margin value. */
  QTimer timer;       /**< The timer. */

};

#endif
