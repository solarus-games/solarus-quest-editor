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
#include "quest.h"
#include <QIcon>
#include <QLayout>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QTextStream>

/**
 * @brief Creates a text editor.
 * @param quest The quest containing the file.
 * @param file_path Path of the file to open.
 * @param parent The parent object or nullptr.
 * @throws EditorException If the file could not be opened.
 */
TextEditor::TextEditor(Quest& quest, const QString& file_path, QWidget* parent) :
  Editor(quest, file_path, parent) {

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);

  text_widget = new QPlainTextEdit(file_path);
  layout->addWidget(text_widget);

  // Use a monospace font.
  QFont font("DejaVu Sans Mono");
  font.setStyleHint(QFont::TypeWriter);
  font.setPointSize(10);
  setFont(font);

  // Activate syntax coloring for Lua scripts.
  if (quest.is_script(file_path)) {
    new LuaSyntaxHighlighter(text_widget->document());
  }

  text_widget->document()->setModified(false);

  // Open the file.
  if (file_path.isEmpty()) {
    return;
  }

  QFile file(file_path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    throw EditorException(tr("Cannot open file '%1'").arg(file_path));
  }
  QTextStream out(&file);
  out.setCodec("UTF-8");
  text_widget->setPlainText(out.readAll());

}

/**
 * @copydoc Editor::get_title
 */
QString TextEditor::get_title() const {

  QString path = get_file_path();
  QString language_id;

  if (get_quest().is_dialogs_file(path, language_id)) {
    return get_file_name() + " (" + language_id + ')';
  }

  if (get_quest().is_strings_file(path, language_id)) {
    return get_file_name() + " (" + language_id + ')';
  }

  return Editor::get_title();
}

/**
 * @copydoc Editor::get_icon
 */
QIcon TextEditor::get_icon() const {

  QString path = get_file_path();
  Solarus::ResourceType resource_type;
  QString element_id;
  if (get_quest().is_resource_element(path, resource_type, element_id)) {
    QString resource_lua_name = get_quest().get_resources().get_lua_name(resource_type);
    return QIcon(":/images/icon_resource_" + resource_lua_name + ".png");
  }

  if (get_quest().is_map_script(path, element_id)) {
    return QIcon(":/images/icon_resource_map.png");
  }

  if (get_quest().is_dialogs_file(path, element_id)) {
    return QIcon(":/images/icon_resource_language.png");
  }

  if (get_quest().is_dialogs_file(path, element_id)) {
    return QIcon(":/images/icon_resource_language.png");
  }

  if (get_quest().is_script(path)) {
    // A Lua script.
    return QIcon(":/images/icon_script.png");
  }
  return QIcon(":/images/icon_file.png");
}

/**
 * @copydoc Editor::save
 */
void TextEditor::save() {

  QFile file(get_file_path());
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    throw EditorException(tr("Cannot open file '%1' for writing").arg(get_file_name()));
  }

  QTextStream out(&file);
  out.setCodec("UTF-8");
  out << text_widget->toPlainText();
  text_widget->document()->setModified(false);
}

/**
 * @copydoc Editor::confirm_close
 */
bool TextEditor::confirm_close() {

  if (!text_widget->document()->isModified()) {
    return true;
  }

  QMessageBox::StandardButton answer = QMessageBox::question(
        nullptr,
        tr("Save the modifications"),
        tr("File '%1' has been modified. Do you want to save it?").arg(get_file_name()),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save
        );

  switch (answer) {

  case QMessageBox::Save:
    // Save and close.
    save();
    return true;

  case QMessageBox::Discard:
    // Close without saving.
    return true;

  case QMessageBox::Cancel:
  case QMessageBox::Escape:
    // Don't close.
    return false;

  default:
    return false;
  }

}

/**
 * @copydoc Editor::cut
 */
void TextEditor::cut() {

  text_widget->cut();
}

/**
 * @copydoc Editor::copy
 */
void TextEditor::copy() {

  text_widget->copy();
}

/**
 * @copydoc Editor::paste
 */
void TextEditor::paste() {

  text_widget->paste();
}
