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
#include <QPlainTextEdit>

class TextEditor;

/**
 * @brief Customization of QPlainTextEdit to work with QUndoGroup.
 *
 * This implementation allows the undo/redo system of QPlainTextEdit to work
 * with an existing QUndoStack, and therefore with a QUndoGroup if wanted.
 * This is not the case by default, because QPlainTextEdit has its own
 * internal undo/redo implementation.
 *
 * Basically, these internal undo/redo commands are preserved, but the undo
 * and redo actions that trigger them are always under our control.
 * The context menu and the key event are replaced to suppress built-in
 * undo/redo actions and implement our own actions instead using the QUndoStack.
 */
class TextEditorWidget: public QPlainTextEdit {
  Q_OBJECT;

public:

  TextEditorWidget(const QString& file_path, TextEditor& editor);

  void contextMenuEvent(QContextMenuEvent* event);
  void keyPressEvent(QKeyEvent* event);

private slots:

  void undo_command_added();

private:

  QUndoStack& undo_stack;    /**< The undo/redo history to use. */

};
