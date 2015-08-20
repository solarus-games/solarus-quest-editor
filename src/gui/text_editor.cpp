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
#include "gui/find_text_dialog.h"
#include "gui/lua_syntax_highlighter.h"
#include "gui/text_editor.h"
#include "gui/text_editor_widget.h"
#include "editor_exception.h"
#include "quest.h"
#include "settings.h"
#include <QIcon>
#include <QLayout>
#include <QList>
#include <QPlainTextEdit>
#include <QScrollBar>
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

  set_title(create_title());
  set_icon(create_icon());
  set_find_supported(true);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);

  text_widget = new TextEditorWidget(file_path, *this);
  layout->addWidget(text_widget);

  // Open map shorcut.
  if (quest.is_map_script(file_path, map_id)) {
    QAction* open_map_action = new QAction(this);
    open_map_action->setShortcut(tr("F4"));
    open_map_action->setShortcutContext(Qt::WindowShortcut);
    connect(open_map_action, SIGNAL(triggered(bool)),
            this, SLOT(open_map_requested()));
    addAction(open_map_action);
  } else {
    map_id.clear();
  }

  connect(text_widget, SIGNAL(copyAvailable(bool)),
          this, SIGNAL(can_cut_changed(bool)));
  connect(text_widget, SIGNAL(copyAvailable(bool)),
          this, SIGNAL(can_copy_changed(bool)));

  reload_settings();

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
 * @brief Chooses an appropriate title for this editor.
 * @return A title.
 */
QString TextEditor::create_title() const {

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
 * @brief Chooses an appropriate icon for this editor.
 * @return An icon.
 */
QIcon TextEditor::create_icon() const {

  QString path = get_file_path();
  ResourceType resource_type;
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
 * @copydoc Editor::can_cut
 */
bool TextEditor::can_cut() const {
  return !text_widget->isReadOnly() && can_copy();
}

/**
 * @copydoc Editor::cut
 */
void TextEditor::cut() {

  text_widget->cut();
}

/**
 * @copydoc Editor::can_copy
 */
bool TextEditor::can_copy() const {
  return text_widget->textCursor().hasSelection();
}

/**
 * @copydoc Editor::copy
 */
void TextEditor::copy() {

  text_widget->copy();
}

/**
 * @copydoc Editor::can_paste
 */
bool TextEditor::can_paste() const {
  return true;
}

/**
 * @copydoc Editor::paste
 */
void TextEditor::paste() {

  text_widget->paste();
}

/**
 * @copydoc Editor::find
 */
void TextEditor::find() {

  FindTextDialog* dialog = new FindTextDialog(this);

  connect(dialog, SIGNAL(find_text_requested(QString)),
          this, SLOT(find_text_requested(QString)));

  dialog->show();
  dialog->raise();  // Put the dialog on top.
  dialog->activateWindow();
}

/**
 * @copydoc Editor::reload_settings
 */
void TextEditor::reload_settings() {

  Settings settings;

  // Font.
  QFont font(settings.get_value_string(Settings::font_family));
  font.setPointSize(settings.get_value_int(Settings::font_size));
  font.setStyleHint(QFont::TypeWriter);
  setFont(font);
}

/**
 * @brief Slot called when the user searches an occurence of some text.
 * @param text The text to find.
 */
void TextEditor::find_text_requested(const QString& text) {

  if (!text_widget->find(text)) {
    // Text not found: search back from the beginning.
    QTextCursor cursor = text_widget->textCursor();
    int scroll_x = text_widget->horizontalScrollBar()->value();
    int scroll_y = text_widget->verticalScrollBar()->value();
    text_widget->moveCursor(QTextCursor::Start);
    if (!text_widget->find(text)) {
      // Still not found: restore the cursor position and scrollbars.
      text_widget->setTextCursor(cursor);
      text_widget->horizontalScrollBar()->setValue(scroll_x);
      text_widget->verticalScrollBar()->setValue(scroll_y);
    }
  }
}

/**
 * @brief Slot called when the user wants to open the map view of this script.
 */
void TextEditor::open_map_requested() {

  if (!map_id.isEmpty()) {
    emit open_file_requested(
      get_quest(), get_quest().get_map_data_file_path(map_id));
  }
}
