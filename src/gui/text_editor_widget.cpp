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
#include "gui/text_editor_widget.h"
#include <QMenu>
#include <QUndoStack>

namespace {

/**
 * @brief Undo command that acts as proxy to trigger undo/redo on a text widget.
 *
 * This class is needed because QPlainTextWidget does not know the QUndoCommand
 * framework (it has its own undo/redo implementation instead).
 */
class UndoCommandProxy : public QUndoCommand {

public:

  /**
   * @brief Creates an undo command proxy.
   * @param text_editor_widget The text editor widget to forward undo/redo
   * commands to.
   */
  UndoCommandProxy(TextEditorWidget& text_editor_widget):
    QUndoCommand("text"),
    text_editor_widget(text_editor_widget),
    first_time(true) {

  }

  /**
   * @brief Forwards an undo command to the text editor.
   */
  virtual void undo() override {

    text_editor_widget.undo();
  }

  /**
   * @brief Forwards an redo command to the text editor.
   *
   * Does nothing the first time, because it is not a real redo,
   * it corresponds to the initial action.
   * This initial action is done before creating the UndoCommandProxy.
   */
  virtual void redo() override {

    if (first_time) {
      // Not a real redo.
      first_time = false;
      return;
    }

    text_editor_widget.redo();
  }

private:

  TextEditorWidget& text_editor_widget;     /**< The text editor widget to
                                             * forward undo/redo commands to. */
  bool first_time;                          /**< \c true if redo has not been called yet.*/
};

}

/**
 * @brief Constructor.
 * @param file_path Path of the file to open.
 * @param editor The parent object.
 */
TextEditorWidget::TextEditorWidget(const QString& file_path, TextEditor& editor):
  QPlainTextEdit(file_path, &editor),
  undo_stack(editor.get_undo_stack()) {

  connect(document(), SIGNAL(undoCommandAdded()),
          this, SLOT(undo_command_added()));
}

/**
 * @brief Slot called when the text edit creates an internal undo command.
 *
 * This is called for example when the user enters or deletes some text.
 * A special command is added to the undo stack to reflect it.
 */
void TextEditorWidget::undo_command_added() {

  undo_stack.push(new UndoCommandProxy(*this));
}

/**
 * @brief Shows a context menu.
 *
 * Reimplemented to replace the built-in undo/redo actions by actions that
 * pass through the QUndoStack.
 *
 * @param event The context menu event.
 */
void TextEditorWidget::contextMenuEvent(QContextMenuEvent* event) {

  /* TODO create our own context menu with correct undo/redo actions.
  QMenu menu;

  menu.exec(event->globalPos());
  */
}

/**
 * @brief Receives a key press event.
 *
 * This overrides the implementation of QPlainTextEdit to block the built-in
 * undo/redo actions and use our own ones instead, to pass through the
 * QUndoStack.
 *
 * @param event The event to handle.
 */
void TextEditorWidget::keyPressEvent(QKeyEvent* event) {

  if (event == QKeySequence::Undo) {
    undo_stack.undo();
    event->accept();
    return;
  }

  if (event == QKeySequence::Redo) {
    undo_stack.redo();
    event->accept();
    return;
  }

  QPlainTextEdit::keyPressEvent(event);
}

