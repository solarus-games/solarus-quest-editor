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
#include "gui/quest_tree_view.h"
#include "gui/quest_files_model.h"
#include <QContextMenuEvent>
#include <QFileSystemModel>
#include <QMenu>
#include <QSignalMapper>

/**
 * @brief Creates a quest tree view.
 * @param parent The parent or nullptr.
 */
QuestTreeView::QuestTreeView(QWidget* parent) :
  QTreeView(parent),
  model(nullptr) {

  setUniformRowHeights(true);

  connect(this, SIGNAL(doubleClicked(const QModelIndex&)),
          this, SLOT(item_double_clicked(const QModelIndex&)));
}

/**
 * @brief Sets the quest manager observed by this view.
 * @param quest_manager The quest manager.
 */
void QuestTreeView::set_quest_manager(QuestManager& quest_manager) {

  // TODO replace by set_quest. This will allow several quest trees to live
  // at the same time.

  // Connect to the quest manager.
  connect(&quest_manager, SIGNAL(current_quest_changed(Quest&)),
          this, SLOT(current_quest_changed(Quest&)));
}

/**
 * @brief Slot called when the user loads another quest.
 * @param quest The current quest.
 */
void QuestTreeView::current_quest_changed(Quest& quest) {

  // Clean the old tree.
  setModel(nullptr);
  setSortingEnabled(false);

  // Create a new model.
  model = new QuestFilesModel(quest);
  setModel(model);
  setRootIndex(model->get_quest_root_index());

  if (model->hasChildren(rootIndex())) {
    expand(rootIndex().child(0, 0));  // Expand the data directory.
  }

  sortByColumn(0, Qt::AscendingOrder);
  setColumnWidth(QuestFilesModel::FILE_COLUMN, 200);
  setColumnWidth(QuestFilesModel::DESCRIPTION_COLUMN, 200);

  // It is better for performance to enable sorting only after the model is ready.
  setSortingEnabled(true);
}

/**
 * @brief Slot called when the user double-clicks an item.
 * @param index Index of the item.
 */
void QuestTreeView::item_double_clicked(const QModelIndex& index) {

  QString path = model->get_file_path(index);
  emit open_file_requested(model->get_quest(), path);
}

/**
 * @brief Shows a popup menu with actions related to the selected item.
 * @param event The event to handle.
 */
void QuestTreeView::contextMenuEvent(QContextMenuEvent* event) {

  using ResourceType = Solarus::ResourceType;

  QPoint position = event->pos();
  QModelIndex index = indexAt(position);
  if (!index.isValid()) {
    return;
  }

  Quest& quest = model->get_quest();
  if (!quest.is_valid()) {
    return;
  }

  QMenu* menu = new QMenu(this);
  QAction* open_action = nullptr;
  QSignalMapper* signal_mapper = new QSignalMapper();  // To add the path parameter.
  connect(signal_mapper, SIGNAL(mapped(const QString&)),
          this, SLOT(open_action_triggered(const QString&)));

  QString path = model->get_file_path(index);
  Solarus::ResourceType resource_type;
  QString element_id;
  if (quest.is_resource_element(path, resource_type, element_id)) {
    // A resource element.

    QString resource_type_lua_name = QuestResources::get_lua_name(resource_type);
    QString resource_type_friendly_name = QuestResources::get_friendly_name(resource_type);
    QIcon icon(":/images/icon_resource_" + resource_type_lua_name + ".png");

    switch (resource_type) {

    case ResourceType::MAP:

      // For a map, the user can open the map data file or the map script.
      open_action = new QAction(icon, "Open Map", this);
      connect(open_action, SIGNAL(triggered()),
              signal_mapper, SLOT(map()));
      signal_mapper->setMapping(open_action, path);
      menu->addAction(open_action);

      open_action = new QAction(
            QIcon(":/images/icon_script.png"),
            "Open Map script",
            this);
      connect(open_action, SIGNAL(triggered()),
              signal_mapper, SLOT(map()));
      signal_mapper->setMapping(open_action,
                                path.replace(path.length() - 4, 4, ".lua"));
      menu->addAction(open_action);
      break;

    case ResourceType::LANGUAGE:

      // For a language, the user can open dialogs or strings.
      open_action = new QAction(icon, "Open Dialogs", this);
      connect(open_action, SIGNAL(triggered()),
              signal_mapper, SLOT(map()));
      signal_mapper->setMapping(open_action, path + "/text/dialogs.dat");
      menu->addAction(open_action);

      open_action = new QAction(icon, "Open Strings", this);
      connect(open_action, SIGNAL(triggered()),
              signal_mapper, SLOT(map()));
      signal_mapper->setMapping(open_action, path + "/text/strings.dat");
      menu->addAction(open_action);
      break;

    case ResourceType::TILESET:
    case ResourceType::SPRITE:
    case ResourceType::ITEM:
    case ResourceType::ENEMY:
    case ResourceType::ENTITY:
      // Other editable resource types,
      open_action = new QAction(icon, "Open " + resource_type_friendly_name, this);
      connect(open_action, SIGNAL(triggered()),
              signal_mapper, SLOT(map()));
      signal_mapper->setMapping(open_action, path);
      menu->addAction(open_action);
      break;

    case ResourceType::MUSIC:
    case ResourceType::SOUND:
    case ResourceType::FONT:
      // These resource types cannot be edited.
      break;
    }

  }

  if (menu->isEmpty()) {
    delete menu;
  }
  else {
    menu->popup(viewport()->mapToGlobal(position));
  }
}

/**
 * @brief Slot called when the user wants to open a file.
 * @param path Path of the file to open.
 */
void QuestTreeView::open_action_triggered(const QString& path) {

  emit open_file_requested(model->get_quest(), path);
}
