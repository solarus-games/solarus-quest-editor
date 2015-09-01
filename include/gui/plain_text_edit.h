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
#include <cmath>

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
    changed(false),
    show_margin(false) {

    setTabChangesFocus(true);
    setLineWrapMode(LineWrapMode::NoWrap);

    QTextCharFormat char_format = currentCharFormat();
    char_format.setFontFixedPitch(true);
    setCurrentCharFormat(char_format);

    connect(this, SIGNAL(textChanged()), this, SLOT(handle_text_changed()));
  }

  inline void set_show_margin(bool show_margin, int margin = 0) {

    this->show_margin = show_margin && margin > 0;
    this->margin = margin;
    viewport()->repaint();
  }

signals:

  void editing_finished();

protected:

    inline virtual void focusOutEvent(QFocusEvent* event) override {
      if (changed) {
        emit editing_finished();
        changed = false;
      }
      QPlainTextEdit::focusOutEvent(event);
    }

    inline virtual void paintEvent(QPaintEvent* event) override {

      if (show_margin) {
        const QRect rect = event->rect();
        const QFont font = currentCharFormat().font();
        int x = std::round(QFontMetrics(font).maxWidth() * margin)
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
    changed = true;
  }

private:

  bool changed;
  bool show_margin;
  int margin;

};

#endif
