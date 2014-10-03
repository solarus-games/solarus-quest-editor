/*
 * Copyright (C) 2014 Christopho, Solarus - http://www.solarus-games.org
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
#include "gui/lua_syntax_highlighter.h"
#include "gui/text_editor.h"
#include "editor_exception.h"
#include <QLayout>
#include <QPlainTextEdit>
#include <QTextStream>

/**
 * @brief Creates a text editor.
 * @param quest The quest containing the file.
 * @param file_path Path of the file to open.
 * @param parent The parent object or nullptr.
 * @throws QuestEditorException If the file could not be opened.
 */
TextEditor::TextEditor(Quest& quest, const QString& file_path, QWidget* parent) :
  Editor(quest, file_path, parent) {

  QPlainTextEdit* text_edit = new QPlainTextEdit(file_path);
  layout()->addWidget(text_edit);

  // Use a monospace font.
  QFont font("DejaVu Sans Mono");
  font.setStyleHint(QFont::TypeWriter);
  font.setPointSize(10);
  setFont(font);

  // Activate syntax coloring for Lua scripts.
  if (file_path.endsWith(".lua")) {
    new LuaSyntaxHighlighter(text_edit->document());
  }

  if (file_path.isEmpty()) {
    return;
  }

  QFile file(file_path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    throw EditorException("Cannot open file '" + file_path + "'");
  }

  QTextStream out(&file);
  out.setCodec("UTF-8");
  text_edit->setPlainText(out.readAll());
}
