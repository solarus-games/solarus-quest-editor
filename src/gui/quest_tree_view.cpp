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
#include <gui/gui_tools.h>
#include "gui/quest_tree_view.h"
#include "gui/quest_files_model.h"
#include <QContextMenuEvent>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QSignalMapper>

using ResourceType = Solarus::ResourceType;

/**
 * @brief Creates a quest tree view.
 * @param parent The parent or nullptr.
 */
QuestTreeView::QuestTreeView(QWidget* parent) :
  QTreeView(parent),
  model(nullptr) {

  setUniformRowHeights(true);

  delete_action = new QAction(
        QIcon(":/images/icon_delete.png"), tr("Delete..."), this);
  delete_action->setShortcut(QKeySequence::Delete);
  connect(delete_action, SIGNAL(triggered()),
          this, SLOT(delete_action_triggered()));
  addAction(delete_action);

  // TODO other actions (new, rename, etc.) should also be here.
  // This allows to make keyboard shortcuts.
  // It is also much simpler to use the selected index instead of
  // making a QSignalWatcher to add a path parameter.
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
 * @brief Receives a double-click event.
 * @param event The event to handle.
 */
void QuestTreeView::mouseDoubleClickEvent(QMouseEvent* event) {

  QModelIndex index = indexAt(event->pos());
  if (index.isValid() &&
      index.column() == QuestFilesModel::FILE_COLUMN &&
      !model->hasChildren(index) &&
      event->button() == Qt::LeftButton) {
    // Left double-click on leaf item: open the file.
    QString path = model->get_file_path(index);
    emit open_file_requested(model->get_quest(), path);
    return;
  }

  // Other cases: keep the default double-click behavior
  // (like expanding nodes).
  QTreeView::mouseDoubleClickEvent(event);
}

/**
 * @brief Shows a popup menu with actions related to the selected item.
 * @param event The event to handle.
 */
void QuestTreeView::contextMenuEvent(QContextMenuEvent* event) {

  QPoint position = event->pos();
  QModelIndex index = indexAt(position);
  if (!index.isValid()) {
    return;
  }

  Quest& quest = model->get_quest();
  if (!quest.is_valid()) {
    return;
  }

  QString path = model->get_file_path(index);
  QMenu* menu = new QMenu(this);

  build_context_menu_new(*menu, path);
  build_context_menu_open(*menu, path);
  build_context_menu_rename(*menu, path);
  build_context_menu_delete(*menu, path);

  if (menu->isEmpty()) {
    delete menu;
  }
  else {
    menu->popup(viewport()->mapToGlobal(position));
  }
}

/**
 * @brief Builds the "New" part of a context menu for a file.
 * @param menu The context menu being created.
 * @param path Path whose context menu is requested.
 */
void QuestTreeView::build_context_menu_new(QMenu& menu, const QString& path) {

  if (!menu.isEmpty()) {
    menu.addSeparator();
  }

  Quest& quest = model->get_quest();
  const QuestResources& resources = quest.get_resources();
  QAction* action = nullptr;

  Solarus::ResourceType resource_type;
  if (quest.is_resource_path(path, resource_type) ||
      (quest.is_in_resource_path(path, resource_type) && QFileInfo(path).isDir())) {
    // Resource directory or subdirectory: let the user create a new element in it.
    QSignalMapper* new_element_signal_mapper = new QSignalMapper(this);  // To add the path parameter.
    connect(new_element_signal_mapper, SIGNAL(mapped(const QString&)),
            this, SLOT(new_element_action_triggered(const QString&)));

    QString resource_type_lua_name = resources.get_lua_name(resource_type);
    QString resource_type_friendly_name = resources.get_friendly_name(resource_type);
    action = new QAction(
          QIcon(":/images/icon_resource_" + resource_type_lua_name + ".png"),
          tr("Create %1...").arg(resource_type_friendly_name),
          this);
    connect(action, SIGNAL(triggered()),
            new_element_signal_mapper, SLOT(map()));
    new_element_signal_mapper->setMapping(action, path);
    menu.addAction(action);
    menu.addSeparator();
  }

  if (QFileInfo(path).isDir()) {
    // Any directory: create directory and create script.
    QSignalMapper* new_directory_signal_mapper = new QSignalMapper(this);
    connect(new_directory_signal_mapper, SIGNAL(mapped(const QString&)),
            this, SLOT(new_directory_action_triggered(const QString&)));
    action = new QAction(
          QIcon(":/images/icon_folder_closed.png"),
          tr("New folder..."),
          this);
    connect(action, SIGNAL(triggered()),
            new_directory_signal_mapper, SLOT(map()));
    new_directory_signal_mapper->setMapping(action, path);
    menu.addAction(action);

    QSignalMapper* new_script_signal_mapper = new QSignalMapper(this);
    connect(new_script_signal_mapper, SIGNAL(mapped(const QString&)),
            this, SLOT(new_script_action_triggered(const QString&)));
    action = new QAction(
          QIcon(":/images/icon_script.png"),
          tr("New script..."),
          this);
    connect(action, SIGNAL(triggered()),
            new_script_signal_mapper, SLOT(map()));
    new_script_signal_mapper->setMapping(action, path);
    menu.addAction(action);
  }
}

/**
 * @brief Builds the "Open" part of a context menu for a file.
 * @param menu The context menu being created.
 * @param path Path whose context menu is requested.
 */
void QuestTreeView::build_context_menu_open(QMenu& menu, const QString& path) {

  if (!menu.isEmpty()) {
    menu.addSeparator();
  }

  Quest& quest = model->get_quest();
  const QuestResources& resources = quest.get_resources();
  QAction* action = nullptr;
  QSignalMapper* signal_mapper = new QSignalMapper(this);  // To add the path parameter.
  connect(signal_mapper, SIGNAL(mapped(const QString&)),
          this, SLOT(open_action_triggered(const QString&)));

  Solarus::ResourceType resource_type;
  QString element_id;
  if (quest.is_resource_element(path, resource_type, element_id)) {
    // A resource element.

    QString resource_type_lua_name = resources.get_lua_name(resource_type);
    QIcon icon(":/images/icon_resource_" + resource_type_lua_name + ".png");

    switch (resource_type) {

    case ResourceType::MAP:

      // For a map, the user can open the map data file or the map script.
      action = new QAction(icon, tr("Open"), this);
      connect(action, SIGNAL(triggered()),
              signal_mapper, SLOT(map()));
      signal_mapper->setMapping(action, path);
      menu.addAction(action);

      action = new QAction(
            QIcon(":/images/icon_script.png"),
            tr("Open Script"),
            this);
      connect(action, SIGNAL(triggered()),
              signal_mapper, SLOT(map()));
      signal_mapper->setMapping(action, quest.get_map_script_path(element_id));
      menu.addAction(action);
      break;

    case ResourceType::LANGUAGE:

      // For a language, the user can open dialogs or strings.
      action = new QAction(icon, tr("Open Dialogs"), this);
      connect(action, SIGNAL(triggered()),
              signal_mapper, SLOT(map()));
      signal_mapper->setMapping(action, quest.get_dialogs_path(element_id));
      menu.addAction(action);

      action = new QAction(icon, tr("Open Strings"), this);
      connect(action, SIGNAL(triggered()),
              signal_mapper, SLOT(map()));
      signal_mapper->setMapping(action, quest.get_strings_path(element_id));
      menu.addAction(action);
      break;

    case ResourceType::TILESET:
    case ResourceType::SPRITE:
    case ResourceType::ITEM:
    case ResourceType::ENEMY:
    case ResourceType::ENTITY:
      // Other editable resource types,
      action = new QAction(icon, tr("Open"), this);
      connect(action, SIGNAL(triggered()),
              signal_mapper, SLOT(map()));
      signal_mapper->setMapping(action, path);
      menu.addAction(action);
      break;

    case ResourceType::MUSIC:
    case ResourceType::SOUND:
    case ResourceType::FONT:
      // These resource types cannot be edited.
      break;
    }

  }
  else if (path.endsWith(".lua")) {
    // Open a Lua script that is not a resource.
    action = new QAction(
          QIcon(":/images/icon_script.png"), tr("Open"), this);
    connect(action, SIGNAL(triggered()),
            signal_mapper, SLOT(map()));
    signal_mapper->setMapping(action, path);
    menu.addAction(action);
  }
}

/**
 * @brief Builds the "Rename" part of a context menu for a file.
 * @param menu The context menu being created.
 * @param path Path whose context menu is requested.
 */
void QuestTreeView::build_context_menu_rename(QMenu& menu, const QString& path) {

  if (!menu.isEmpty()) {
    menu.addSeparator();
  }

  Quest& quest = model->get_quest();
  QAction* action = nullptr;

  if (path == quest.get_data_path()) {
    // We don't to rename the data directory.
    return;
  }

  Solarus::ResourceType resource_type;
  QString element_id;
  if (quest.is_resource_path(path, resource_type)) {
    // Nothing good can come from renaming built-in resource directories.
    return;
  }

  // All other paths can have a "Rename" menu item.
  QSignalMapper* rename_signal_mapper = new QSignalMapper(this);
  connect(rename_signal_mapper, SIGNAL(mapped(const QString&)),
          this, SLOT(rename_action_triggered(const QString&)));

  action = new QAction(QIcon(":/images/icon_rename.png"), tr("Rename..."), this);
  connect(action, SIGNAL(triggered()),
          rename_signal_mapper, SLOT(map()));
  rename_signal_mapper->setMapping(action, path);
  menu.addAction(action);

  if (quest.is_resource_element(path, resource_type, element_id)) {
    // Resource element: additionally, allow to change the description.
    QSignalMapper* change_description_signal_mapper = new QSignalMapper(this);
    connect(change_description_signal_mapper, SIGNAL(mapped(const QString&)),
            this, SLOT(change_description_action_triggered(const QString&)));

    action = new QAction(tr("Change description..."), this);
    connect(action, SIGNAL(triggered()),
            change_description_signal_mapper, SLOT(map()));
    change_description_signal_mapper->setMapping(action, path);
    menu.addAction(action);
  }
}

/**
 * @brief Builds the "Delete" part of a context menu for a file.
 * @param menu The context menu being created.
 * @param path Path whose context menu is requested.
 */
void QuestTreeView::build_context_menu_delete(QMenu& menu, const QString& path) {

  if (!menu.isEmpty()) {
    menu.addSeparator();
  }

  Quest& quest = model->get_quest();

  if (path == quest.get_data_path()) {
    // We don't want to delete the data directory.
    return;
  }

  Solarus::ResourceType resource_type;
  if (quest.is_resource_path(path, resource_type)) {
    // Don't delete resource directories.
    return;
  }

  // All other paths can have a "Delete" menu item.
  menu.addAction(delete_action);
}

/**
 * @brief Slot called when the user wants to open a file.
 * @param path Path of the file to open.
 */
void QuestTreeView::open_action_triggered(const QString& path) {

  emit open_file_requested(model->get_quest(), path);
}

/**
 * @brief Slot called when the user wants to create a new resource element.
 *
 * The id and description of the element will be prompted to the user.
 *
 * @param parent_path Directory where the new element will be created.
 * The type of resource is indicated by the beginning of this path.
 */
void QuestTreeView::new_element_action_triggered(const QString& parent_path) {
  // TODO
}

/**
 * @brief Slot called when the user wants to create a new directory.
 *
 * The directory name will be prompted to the user.
 *
 * @param parent_path An exiting directory where the new one will be created.
 */
void QuestTreeView::new_directory_action_triggered(const QString& parent_path) {
  // TODO
}

/**
 * @brief Slot called when the user wants to create a new Lua script.
 *
 * The file name will be prompted to the user.
 *
 * @param parent_path An exiting directory where the new script will be created.
 */
void QuestTreeView::new_script_action_triggered(const QString& parent_path) {
  // TODO
}

/**
 * @brief Slot called when the user wants to rename a file or directory.
 *
 * The new name will be prompted to the user.
 *
 * @param path The path to rename.
 */
void QuestTreeView::rename_action_triggered(const QString& path) {
  // TODO
}

/**
 * @brief Slot called when the user wants to change the description of a
 * resource element.
 *
 * The new description will be prompted to the user.
 *
 * @param path The path of the resource to change.
 */
void QuestTreeView::change_description_action_triggered(const QString& path) {

  Quest& quest = model->get_quest();
  QuestResources& resources = quest.get_resources();
  ResourceType resource_type;
  QString element_id;
  if (!quest.is_resource_element(path, resource_type, element_id)) {
    // Only resource elements have a description.
    return;
  }

  QString resource_friendly_type_name_for_id = resources.get_friendly_name_for_id(resource_type);
  QString old_description = resources.get_description(resource_type, element_id);
  bool ok;
  QString new_description = QInputDialog::getText(
        this,
        tr("Change description"),
        tr("New description for  %1 '%2':").arg(resource_friendly_type_name_for_id, element_id),
        QLineEdit::Normal,
        old_description,
        &ok);

  if (ok && !new_description.isEmpty()) {
    resources.set_description(resource_type, element_id, new_description);
    resources.save();
  }
}

/**
 * @brief Slot called when the user wants to delete the selected file or
 * directory.
 *
 * Confirmation will be asked to the user.
 */
void QuestTreeView::delete_action_triggered() {

  QModelIndexList selected_indexes = selectedIndexes();
  if (selected_indexes.isEmpty()) {
    return;
  }
  const QString& path = model->get_file_path(selected_indexes.first());
  Quest& quest = model->get_quest();
  if (path == quest.get_data_path()) {
    // We don't delete the data directory.
    return;
  }
  Solarus::ResourceType resource_type;
  if (quest.is_resource_path(path, resource_type)) {
    // Don't delete resource directories.
    return;
  }

  // See if we want to delete
  // - a resource element,
  // - a directory,
  // - a file.
  QString path_from_data = path.right(path.length() - quest.get_data_path().length() - 1);
  QString element_id;
  if (quest.is_resource_element(path, resource_type, element_id)) {
    // This is a resource element.

    QuestResources& resources = quest.get_resources();
    const QString& resource_friendly_name_for_id =
        resources.get_friendly_name_for_id(resource_type);
    QMessageBox::StandardButton answer = QMessageBox::question(
          this,
          tr("Delete confirmation"),
          tr("Do you really want to delete %1 '%2'?").
                 arg(resource_friendly_name_for_id).arg(element_id),
          QMessageBox::Yes | QMessageBox::No);

    if (answer != QMessageBox::Yes) {
      return;
    }

    bool success = false;
    if (QFileInfo(path).isDir()) {
      // Some resource elements are directories (languages).
      success = QDir(path).removeRecursively();
    }
    else {
      // Remove the main resource file.
      success = QFile(path).remove();

      // Some resource type have additional files.
      QFile additional_file;
      switch (resource_type) {

      case ResourceType::MAP:
        // Also delete the map script.
        additional_file.setFileName(quest.get_map_script_path(element_id));
        if (additional_file.exists()) {
          success &= additional_file.remove();
        }
        break;

      case ResourceType::TILESET:
        // Also delete the tileset images.
        additional_file.setFileName(quest.get_tileset_tiles_image_path(element_id));
        if (additional_file.exists()) {
          success &= additional_file.remove();
        }
        additional_file.setFileName(quest.get_tileset_entities_image_path(element_id));
        if (additional_file.exists()) {
          success &= additional_file.remove();
        }
        break;

      default:
        break;
      }
    }

    if (!success) {
      return;
    }

    // The file was successfully removed from the filesystem.
    // Also temove it from the resource list.
    resources.remove(resource_type, element_id);
    resources.save();
  }

  else {
    // This is a regular file or directory.
    if (QFileInfo(path).isDir()) {
      QDir dir(path);
      bool empty_dir = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count() == 0;
      if (!empty_dir) {
        // TODO we could remove directoriers recursively,
        // but we have to take of resources they contain.
        GuiTools::warningDialog(tr("Directory is not empty"));
      }
      else {
        // Empty directory.
        QMessageBox::StandardButton answer = QMessageBox::question(
              this,
              tr("Delete confirmation"),
              tr("Do you really want to delete folder '%1'?").arg(path_from_data),
              QMessageBox::Yes | QMessageBox::No);

        if (answer != QMessageBox::Yes) {
          return;
        }
        QDir(path + "/..").rmdir(dir.dirName());
      }
    }
    else {
      // Not a directory and not a resource.
      QMessageBox::StandardButton answer = QMessageBox::question(
            this,
            tr("Delete confirmation"),
            tr("Do you really want to delete file '%1'?").arg(path_from_data),
            QMessageBox::Yes | QMessageBox::No);

      if (answer != QMessageBox::Yes) {
        return;
      }
      QFile(path).remove();
    }
  }
}
