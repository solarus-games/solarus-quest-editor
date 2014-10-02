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
#include "gui/text_editor.h"
#include <QLayout>
#include <QPlainTextEdit>

/**
 * @brief Creates a text editor.
 * @param file_name Name of the file to open or an empty string.
 * @param parent The parent object or nullptr.
 */
TextEditor::TextEditor(const QString& file_name, QWidget* parent) :
  Editor(parent) {

  layout()->addWidget(new QPlainTextEdit(file_name));

  QFont font("Monospace");
  font.setStyleHint(QFont::TypeWriter);

  setFont(font);

}
