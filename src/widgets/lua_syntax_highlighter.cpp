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
#include "widgets/lua_syntax_highlighter.h"
#include <QVariant>

namespace SolarusEditor {

/**
 * @brief Creates a Lua syntax highlighter.
 * @param document The text document to highlight.
 */
LuaSyntaxHighlighter::LuaSyntaxHighlighter(QTextDocument* document) :
    QSyntaxHighlighter(document) {

  HighlightingRule rule;

  // The following awful regexp beginning can be used to match things that are
  // not in a string.
  // To do that, we check that the number of quotes and double-quotes is even.
  // (Keeping state information would probably be more readable.)
  QString not_in_a_single_line_string = "^[^\"']*(?:\"[^\"]*\"[^\"'']*|\'[^\']*\'[^\"']*)*";

  // Keywords.
  keyword_format.setForeground(Qt::darkRed);
  keyword_format.setFontWeight(QFont::Bold);
  QStringList keyword_patterns;
  keyword_patterns << "\\band\\b"
                   << "\\bbreak\\b"
                   << "\\bdo\\b"
                   << "\\belse\\b"
                   << "\\belseif\\b"
                   << "\\bend\\b"
                   << "\\bfalse\\b"
                   << "\\bfor\\b"
                   << "\\bfunction\\b"
                   << "\\bif\\b"
                   << "\\bin\\b"
                   << "\\blocal\\b"
                   << "\\bnil\\b"
                   << "\\bnot\\b"
                   << "\\bor\\b"
                   << "\\brepeat\\b"
                   << "\\breturn\\b"
                   << "\\bthen\\b"
                   << "\\btrue\\b"
                   << "\\buntil\\b"
                   << "\\bwhile\\b";

  Q_FOREACH (const QString& pattern, keyword_patterns) {
    rule.pattern = QRegExp(pattern);
    rule.format = keyword_format;
    rules.append(rule);
  }

  // Strings.
  string_format.setForeground(Qt::blue);
  rule.pattern = QRegExp("\"[^\"]*\"");
  rule.format = string_format;
  rules.append(rule);

  rule.pattern = QRegExp("'[^']*'");
  rule.format = string_format;
  rules.append(rule);

  // Comments.
  single_line_comment_format.setForeground(Qt::darkGreen);
  // Avoid to highlight comments in strings,
  // and don't match --[[ or --]] markers.
  rule.pattern = QRegExp(
        not_in_a_single_line_string +
        R"((--([^\[\]]|\[[^\[]|\][^\]])[^\n]*$)$)"
  );
  rule.format = single_line_comment_format;
  rules.append(rule);

  comment_start_pattern = QRegExp("--\\[\\[");
  comment_end_pattern = QRegExp("--\\]\\]");
  multi_line_comment_format.setForeground(
        single_line_comment_format.foreground());
}

/**
 * @brief Highlights part of the document.
 * @param text The text to highlight.
 */
void LuaSyntaxHighlighter::highlightBlock(const QString& text) {

  Q_FOREACH (const HighlightingRule& rule, rules) {
    QRegExp pattern(rule.pattern);
    int index = pattern.indexIn(text);
    if (pattern.captureCount() > 0) {
      index = pattern.pos(1);
    }
    while (index >= 0) {
      int length = pattern.matchedLength();
      if (pattern.captureCount() > 0) {
        length = pattern.capturedTexts()[0].size();
      }
      setFormat(index, length, rule.format);
      index = pattern.indexIn(text, index + length);
    }
  }
  setCurrentBlockState(0);

  int start_index = 0;
  if (previousBlockState() != 1) {
    start_index = comment_start_pattern.indexIn(text);
  }

  while (start_index >= 0) {
    int end_index = comment_end_pattern.indexIn(text, start_index);
    int comment_length;
    if (end_index == -1) {
      setCurrentBlockState(1);
      comment_length = text.length() - start_index;
    } else {
      comment_length = end_index - start_index
          + comment_end_pattern.matchedLength();
    }
    setFormat(start_index, comment_length, multi_line_comment_format);
    start_index = comment_start_pattern.indexIn(text, start_index + comment_length);
  }
}

}
