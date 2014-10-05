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
#include "gui/closable_tab_bar.h"
#include "gui/editor_tabs.h"
#include "gui/gui_tools.h"
#include "gui/text_editor.h"
#include "editor_exception.h"
#include "quest.h"

/**
 * @brief Creates an editor tab widget.
 * @param parent The parent object or nullptr.
 */
EditorTabs::EditorTabs(QWidget* parent):
  QTabWidget(parent) {

  setMovable(true);

  ClosableTabBar* tab_bar = new ClosableTabBar();
  setTabBar(tab_bar);
  connect(tab_bar, SIGNAL(tabCloseRequested(int)),
          this, SLOT(close_file_requested(int)));
}

/**
 * @brief Sets the quest manager observed by this view.
 * @param quest_manager The quest manager.
 */
void EditorTabs::set_quest_manager(QuestManager& /* quest_manager */) {
  // Not used for now.
  // Tabs are independant of the current quest.
}

/**
 * @brief Shows a tab to edit the specified resource element.
 * @param quest A Solarus quest.
 * @param resource_type A type of resource.
 * @param id Id of the resource element to open in this quest.
 */
void EditorTabs::open_resource(
    Quest& quest, ResourceType resource_type, const QString& id) {

  switch (resource_type) {

  case ResourceType::MAP:
    // Open the map data file.
    open_map_editor(quest, quest.get_map_data_file_path(id));
    break;

  case ResourceType::TILESET:
    // Open the tileset data file.
    open_tileset_editor(quest, quest.get_tileset_path(id));
    break;

  case ResourceType::LANGUAGE:
    // Open the dialogs file.
    open_dialogs_editor(quest, quest.get_dialogs_path(id));
    break;

  case ResourceType::SPRITE:
    // Open the sprite file.
    open_sprite_editor(quest, quest.get_sprite_path(id));
    break;

  case ResourceType::ITEM:
    // Open the item script.
    open_text_editor(quest, quest.get_item_script_path(id));
    break;

  case ResourceType::ENEMY:
    // Open the enemy script.
    open_text_editor(quest, quest.get_enemy_script_path(id));
    break;

  case ResourceType::ENTITY:
    // Open the custom entity model script.
    open_text_editor(quest, quest.get_entity_script_path(id));
    break;

  case ResourceType::MUSIC:
  case ResourceType::SOUND:
  case ResourceType::FONT:
    // These resource types cannot be edited.
    break;
  }
}

/**
 * @brief Opens a file with a text editor in a new tab.
 *
 * The file may be a Lua script.
 *
 * @param quest A Solarus quest.
 * @param path Path of the Lua file to open.
 */
void EditorTabs::open_text_editor(
    Quest& quest, const QString& path) {

  if (!quest.is_in_root_path(path)) {
    // Not a file of this quest.
    return;
  }

  // Find the existing tab if any.
  int index = find_editor(path);
  if (index != -1) {
    // Already open.
    setCurrentIndex(index);
    return;
  }

  try {
    add_editor(new TextEditor(quest, path));
  }
  catch (const EditorException& ex) {
    ex.show_dialog();
  }
}

/**
 * @brief Opens a file with a map editor in a new tab.
 * @param quest A Solarus quest.
 * @param path Path of the map data file to open.
 */
void EditorTabs::open_map_editor(
    Quest& quest, const QString& path) {

  // TODO map editor.
}

/**
 * @brief Opens a file with a tileset editor in a new tab.
 * @param quest A Solarus quest.
 * @param path Path of the tileset data file to open.
 */
void EditorTabs::open_tileset_editor(
    Quest& quest, const QString& path) {

  // TODO tileset editor.
}

/**
 * @brief Opens a file with a sprite editor in a new tab.
 * @param quest A Solarus quest.
 * @param path Path of the sprite data file to open.
 */
void EditorTabs::open_sprite_editor(
    Quest& quest, const QString& path) {

  open_text_editor(quest, path);  // TODO sprite editor.
}

/**
 * @brief Opens a file with a language dialogs editor in a new tab.
 * @param quest A Solarus quest.
 * @param path Path of the dialogs file to open.
 */
void EditorTabs::open_dialogs_editor(
    Quest& quest, const QString& path) {

  open_text_editor(quest, path);  // TODO dialogs editor.
}

/**
 * @brief Opens a file with a language strings list editor in a new tab.
 * @param quest A Solarus quest.
 * @param path Path of the strings file to open.
 */
void EditorTabs::open_strings_editor(
    Quest& quest, const QString& path) {

  open_text_editor(quest, path);  // TODO strings list editor.
}

/**
 * @brief Creates a new tab and shows it.
 * @param editor The editor to put in the new tab.
 */
void EditorTabs::add_editor(Editor* editor) {

  QString path = editor->get_file_path();
  editors.insert(path, editor);
  addTab(editor, editor->get_icon(), editor->get_title());
  int index = count() - 1;
  setTabToolTip(index, editor->get_file_path());
  setCurrentIndex(index);

  // Show asterisk in tab title when a file is modified.
  connect(editor, SIGNAL(modification_state_changed(bool)),
          this, SLOT(modification_state_changed(bool)));
}

/**
 * @brief Closes the editor at the specified index without confirmation.
 * @param index An editor index.
 */
void EditorTabs::remove_editor(int index) {

  Editor* editor = get_editor(index);
  editors.remove(editor->get_file_path());
  removeTab(index);
}

/**
 * @brief Returns the editor at the specified index.
 * @param index An editor index.
 * @return The editor at this index in the tab bar or nullptr.
 */
Editor* EditorTabs::get_editor(int index) {

  if (index == -1 || index >= count()) {
    return nullptr;
  }

  return static_cast<Editor*>(widget(index));
}

/**
 * @brief Returns the editor currently open if any
 * @return The current editor.
 * Returns nullptr if there is no tab.
 */
Editor* EditorTabs::get_editor() {

  int index = currentIndex();
  if (index == -1) {
    return nullptr;
  }

  return get_editor(index);
}

/**
 * @brief Returns the index of an editor in the tabs.
 * @param path Path of a file to find the editor of.
 * @return The index of the editor or -1 if the file is not open.
 */
int EditorTabs::find_editor(const QString& path) {

  Editor* editor = editors.value(path);
  if (editor == nullptr) {
    return -1;
  }

  return indexOf(editor);
}

/**
 * @brief If the specified file is open in a tab, sets it as the current tab.
 * @param path Path of a file to show the editor of.
 * @return @c true if the file is open in an editor.
 */
bool EditorTabs::show_editor(const QString& path) {

  Editor* editor = editors.value(path);
  if (editor == nullptr) {
    return false;
  }

  setCurrentWidget(editor);
  return true;
}

/**
 * @brief Slot called when the user attempts to open a file.
 * @param quest The quest that holds this file.
 * @param file_path Path of the file to open.
 */
void EditorTabs::open_file_requested(Quest& quest, const QString& path) {

  if (path.isEmpty()) {
    return;
  }

  if (!quest.is_in_root_path(path)) {
    // Not a file of this quest.
    return;
  }

  Solarus::ResourceType resource_type;
  QString element_id;
  if (quest.is_resource_element(path, resource_type, element_id)) {
    // A resource element declared in the quest.
    // Possibly a map data file, an enemy Lua script,
    // a language directory, etc.
    open_resource(quest, resource_type, element_id);
  }
  else if (quest.is_dialogs_file(path, element_id)) {
    open_dialogs_editor(quest, path);
  }
  else if (quest.is_strings_file(path, element_id)) {
    open_strings_editor(quest, path);
  }
  else if (path.endsWith(".lua")) {
    // A Lua script that is not a resource element.
    open_text_editor(quest, path);
  }
}

/**
 * @brief Slot called when the user attempts to close a tab.
 * @param index Index of the tab to close.
 */
void EditorTabs::close_file_requested(int index) {

  Editor* editor = get_editor(index);
  if (editor != nullptr && editor->confirm_close()) {
    removeTab(index);
  }
}

/**
 * @brief Function called when the user wants to close all editors.
 *
 * The user can save files if necessary.
 *
 * @return @c false to cancel the closing operation.
 */
bool EditorTabs::confirm_close() {

  for (int i = 0; i < count(); ++i) {

    Editor* editor = get_editor(i);
    if (!editor->confirm_close()) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Slot called when the is-modified state of a file has changed.
 * @param modified @c true if the file is now modified, @c false if it is now
 * saved.
 */
void EditorTabs::modification_state_changed(bool modified) {

  Editor* editor = dynamic_cast<Editor*>(sender());
  if (editor == nullptr) {
    return;
  }

  QString title = editor->get_title();
  if (modified) {
    title += '*';
  }
  setTabText(indexOf(editor), title);
}
