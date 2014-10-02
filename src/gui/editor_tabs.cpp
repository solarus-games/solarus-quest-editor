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
#include "gui/editor_tabs.h"

#include <QAction>
#include <QLabel>  // TODO remove

/**
 * @brief Creates the editors tab widget.
 * @param parent The parent object or nullptr.
 */
EditorTabs::EditorTabs(QWidget* parent):
  QTabWidget(parent) {

  addTab(new QLabel("TODO", this),
         QIcon(":/images/icon_resource_map.png"),
         "Map");
  addTab(new QLabel("TODO", this),
         QIcon(":/images/icon_resource_tileset.png"),
         "Tileset");
  addTab(new QLabel("TODO", this),
         QIcon(":/images/icon_script.png"),
         "Script");

  setTabsClosable(true);
  setMovable(true);

  // Remove a tab when clicking its close button.
  connect(this, SIGNAL(tabCloseRequested(int)),
          this, SLOT(on_tab_close_requested(int)));

  connect(this, SIGNAL(tabBarClicked(int)),
          this, SLOT(on_tab_close_requested(int)));
}

/**
 * @brief Slot called when the user attempts to close a tab.
 * @param index Index of the tab to closed.
 */
void EditorTabs::on_tab_close_requested(int index) {

  // TODO confirm save
  removeTab(index);
}
