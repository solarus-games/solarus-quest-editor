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
#include "gui/text_editor.h"
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
  // TODO
}

/**
 * @brief Shows a tab to edit a Lua script file that is not a resource element.
 * @param quest A Solarus quest.
 * @param path Path of the Lua file to open.
 */
void EditorTabs::open_script(
    Quest& quest, const QString& path) {

  // TODO find the existing tab if any. Identify tabs by the full path.
  addTab(new TextEditor(path), QIcon(":/icon_script.png"), path.section('/', -1));
}

/**
 * @brief Slot called when the user attempts to open a file.
 * @param quest The quest that holes this file.
 * @param file_path Path of the file to open.
 */
void EditorTabs::open_file_requested(Quest& quest, const QString& path) {

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

  if (path.endsWith(".lua")) {
    // A Lua script that is not a resource element.
    open_script(quest, path);
  }
}

/**
 * @brief Slot called when the user attempts to close a tab.
 * @param index Index of the tab to closed.
 */
void EditorTabs::close_file_requested(int index) {

  // TODO confirm save
  removeTab(index);
}
