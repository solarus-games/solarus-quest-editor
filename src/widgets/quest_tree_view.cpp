/*
 * Copyright (C) 2014-2016 Christopho, Solarus - http://www.solarus-games.org
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
#include "widgets/gui_tools.h"
#include "widgets/new_resource_element_dialog.h"
#include "widgets/quest_tree_view.h"
#include "editor_exception.h"
#include "quest.h"
#include "quest_files_model.h"
#include "sound.h"
#include <QContextMenuEvent>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>

namespace SolarusEditor {

/**
 * @brief Creates a quest tree view.
 * @param parent The parent or nullptr.
 */
QuestTreeView::QuestTreeView(QWidget* parent) :
  QTreeView(parent),
  model(nullptr) {

  setUniformRowHeights(true);
  setAutoScroll(false);
  setAlternatingRowColors(true);

  play_action = new QAction(
        QIcon(":/images/icon_start.png"), tr("Play"), this);
  play_action->setShortcutContext(Qt::WidgetShortcut);
  connect(play_action, SIGNAL(triggered()),
          this, SLOT(play_action_triggered()));
  addAction(play_action);

  open_action = new QAction(tr("Open"), this);
  open_action->setShortcutContext(Qt::WidgetShortcut);
  connect(open_action, SIGNAL(triggered()),
          this, SLOT(open_action_triggered()));
  addAction(open_action);

  rename_action = new QAction(
        QIcon(":/images/icon_rename.png"), tr("Rename..."), this);
  rename_action->setShortcut(tr("F2"));
  rename_action->setShortcutContext(Qt::WidgetShortcut);
  connect(rename_action, SIGNAL(triggered()),
          this, SLOT(rename_action_triggered()));
  addAction(rename_action);

  delete_action = new QAction(
        QIcon(":/images/icon_delete.png"), tr("Delete..."), this);
  delete_action->setShortcut(QKeySequence::Delete);
  delete_action->setShortcutContext(Qt::WidgetShortcut);
  connect(delete_action, SIGNAL(triggered()),
          this, SLOT(delete_action_triggered()));
  addAction(delete_action);

}

/**
 * @brief Sets the quest to represent in this tree view.
 * @param quest The quest.
 */
void QuestTreeView::set_quest(Quest& quest) {

  // Clean the old tree.
  setModel(nullptr);
  setSortingEnabled(false);

  if (quest.exists()) {
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

    connect(&quest, SIGNAL(file_renamed(QString, QString)),
            this, SLOT(file_renamed(QString, QString)));

    // It is better for performance to enable sorting only after the model is ready.
    setSortingEnabled(true);
  }
}

/**
 * @brief Returns the file path of the item selected if any.
 * @return The selected file path or an empty string.
 */
QString QuestTreeView::get_selected_path() const {

  QModelIndexList selected_indexes = selectedIndexes();
  if (selected_indexes.isEmpty()) {
    return "";
  }
  return model->get_file_path(selected_indexes.first());
}

/**
 * @brief Selects the item corresponding to the specified file path.
 *
 * Also ensures the parent of the item to make it visible.
 *
 * @param The file path to selected. An empty string unselects any previous
 * path.
 */
void QuestTreeView::set_selected_path(const QString& path) {

  if (get_selected_path() == path) {
    return;
  }

  selectionModel()->clear();
  QModelIndex index = model->get_file_index(path);
  if (index.isValid()) {
    selectionModel()->select(index, QItemSelectionModel::Select);
    expand(index.parent());
  }
}

/**
 * @brief Receives a key press event.
 * @param event The event to handle.
 */
void QuestTreeView::keyPressEvent(QKeyEvent* event) {

  if (event->key() == Qt::Key_Enter ||
      event->key() == Qt::Key_Return) {
    default_action_triggered();
  }
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
    // Left double-click on leaf item: open or play the file.
    default_action_triggered();
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

  build_context_menu_play(*menu, path);
  build_context_menu_open(*menu, path);
  build_context_menu_new(*menu, path);
  build_context_menu_rename(*menu, path);
  build_context_menu_delete(*menu, path);

  if (menu->isEmpty()) {
    delete menu;
  }
  else {
    menu->popup(viewport()->mapToGlobal(position) + QPoint(1, 1));
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

  ResourceType resource_type;
  QString element_id;
  const bool is_potential_resource_element = quest.is_potential_resource_element(path, resource_type, element_id);
  const bool is_declared_resource_element = is_potential_resource_element && resources.exists(resource_type, element_id);
  const bool is_dir = QFileInfo(path).isDir();

  QAction* new_resource_element_action = nullptr;

  if (is_potential_resource_element) {

    if (is_declared_resource_element) {
      // No creation item on an existing resource element
      // (even for languages, that are actually directories).
      return;
    }

    // File that looks like a resource element, but that is not declared in
    // the quest: let the user add it.

    QString resource_type_friendly_name = resources.get_friendly_name(resource_type);
    QString resource_type_lua_name = resources.get_lua_name(resource_type);

    new_resource_element_action = new QAction(
          QIcon(":/images/icon_resource_" + resource_type_lua_name + ".png"),
          tr("Add to quest as %1...").arg(resource_type_friendly_name),
          this);
  }
  else if (quest.is_resource_path(path, resource_type) ||
           (is_dir && quest.is_in_resource_path(path, resource_type))) {
    // Resource directory or subdirectory: let the user create a new element in it.

    QString resource_type_create_friendly_name = resources.get_create_friendly_name(resource_type);
    QString resource_type_lua_name = resources.get_lua_name(resource_type);

    new_resource_element_action = new QAction(
          QIcon(":/images/icon_resource_" + resource_type_lua_name + ".png"),
          resource_type_create_friendly_name,
          this);
  }

  if (new_resource_element_action != nullptr) {

    connect(new_resource_element_action, SIGNAL(triggered()),
            this, SLOT(new_element_action_triggered()));
    menu.addAction(new_resource_element_action);
    menu.addSeparator();
  }

  if (is_dir) {
    // Any directory: create directory and create script.

    QAction* action = new QAction(
          QIcon(":/images/icon_folder_closed.png"),
          tr("New folder..."),
          this);
    connect(action, SIGNAL(triggered()),
            this, SLOT(new_directory_action_triggered()));
    menu.addAction(action);

    action = new QAction(
          QIcon(":/images/icon_script.png"),
          tr("New script..."),
          this);
    connect(action, SIGNAL(triggered()),
            this, SLOT(new_script_action_triggered()));
    menu.addAction(action);
  }
}

/**
 * @brief Builds the "Play" part of a context menu for a file.
 * @param menu The context menu being created.
 * @param path Path whose context menu is requested.
 */
void QuestTreeView::build_context_menu_play(QMenu& menu, const QString& path) {

  const Quest& quest = model->get_quest();

  ResourceType resource_type;
  QString element_id;
  if (quest.is_potential_resource_element(path, resource_type, element_id)) {
    // A resource element.

    switch (resource_type) {

    case ResourceType::MUSIC:
    case ResourceType::SOUND:
      play_action->setEnabled(quest.exists(path));
      menu.addAction(play_action);
      break;

    default:
      break;
    }

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
  open_action->setText(tr("Open"));  // Restore the normal Open text and icon.
  open_action->setIcon(QIcon());

  ResourceType resource_type;
  QString element_id;
  if (quest.is_resource_element(path, resource_type, element_id)) {
    // A resource element.

    QString resource_type_lua_name = resources.get_lua_name(resource_type);
    open_action->setIcon(
          QIcon(":/images/icon_resource_" + resource_type_lua_name + ".png"));

    switch (resource_type) {

    case ResourceType::MAP:

      // For a map, the user can open the map data file or the map script.
      menu.addAction(open_action);

      action = new QAction(
            QIcon(":/images/icon_script.png"),
            tr("Open Script"),
            this);
      connect(action, SIGNAL(triggered()),
              this, SLOT(open_map_script_action_triggered()));
      menu.addAction(action);
      break;

    case ResourceType::LANGUAGE:

      // For a language, the user can open dialogs or strings.
      open_action->setText(tr("Open Dialogs"));
      menu.addAction(open_action);

      action = new QAction(open_action->icon(), tr("Open Strings"), this);
      connect(action, SIGNAL(triggered()),
              this, SLOT(open_language_strings_action_triggered()));
      menu.addAction(action);
      break;

    case ResourceType::TILESET:
    case ResourceType::SPRITE:
    case ResourceType::ITEM:
    case ResourceType::ENEMY:
    case ResourceType::ENTITY:
      // Other editable resource types,
      menu.addAction(open_action);
      break;

    case ResourceType::MUSIC:
    case ResourceType::SOUND:
    case ResourceType::FONT:
      // These resource types cannot be edited.
      break;
    }

  }
  else if (quest.is_script(path)) {
    // Open a Lua script that is not a resource.
    open_action->setIcon(QIcon(":/images/icon_script.png"));
    menu.addAction(open_action);
  }
  else if (quest.is_data_path(path)) {
    // Open quest properties file.
    open_action->setText(tr("Open Properties"));
    menu.addAction(open_action);
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
  if (path == quest.get_data_path()) {
    // We don't want to rename the data directory.
    return;
  }

  ResourceType resource_type;
  QString element_id;
  if (quest.is_resource_path(path, resource_type)) {
    // Nothing good can come from renaming built-in resource directories.
    return;
  }

  // All other paths can have a "Rename" menu item.
  menu.addAction(rename_action);

  if (quest.is_resource_element(path, resource_type, element_id)) {
    // Resource element: additionally, allow to change the description.
    QAction* action = new QAction(tr("Change description..."), this);
    connect(action, SIGNAL(triggered()),
            this, SLOT(change_description_action_triggered()));
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

  ResourceType resource_type;
  if (quest.is_resource_path(path, resource_type)) {
    // Don't delete resource directories.
    return;
  }

  // All other paths can have a "Delete" menu item.
  menu.addAction(delete_action);
}

/**
 * @brief Slot called when the user wants to create a new resource element.
 *
 * The type of resource will be the one of the selected element.
 * The id and description of the element will be prompted in a dialog.
 * If the selected element is an existing file, then its id is initially
 * proposed in the dialog.
 */
void QuestTreeView::new_element_action_triggered() {

  QString path = get_selected_path();
  if (path.isEmpty()) {
    return;
  }

  try {
    Quest& quest = model->get_quest();
    QuestResources& resources = quest.get_resources();
    ResourceType resource_type;
    QString initial_id_value;
    QString initial_description_value;

    if (quest.is_potential_resource_element(path, resource_type, initial_id_value)) {
      if (resources.exists(resource_type, initial_id_value)) {
        // The element already exists.
        return;
      }
      initial_description_value = initial_id_value;
    }
    else {
      if (!quest.is_resource_path(path, resource_type) &&
          !quest.is_in_resource_path(path, resource_type)) {
        // We expect a built-in resource directory or a subdirectory or it.
        return;
      }

      // Put the directory clicked as initial id value in the dialog.
      QString resource_path = quest.get_resource_path(resource_type);
      if (path != resource_path) {
        initial_id_value = path.right(path.size() - resource_path.length() - 1) + '/';
      }
      else {
        initial_id_value = "";  // Top-level resource directory.
      }
    }

    NewResourceElementDialog dialog(resource_type, parentWidget());
    dialog.set_element_id(initial_id_value);
    dialog.set_element_description(initial_description_value);
    int result = dialog.exec();

    if (result != QDialog::Accepted) {
      return;
    }

    QString element_id = dialog.get_element_id();
    QString description = dialog.get_element_description();

    model->get_quest().create_resource_element(
          resource_type, element_id, description);

    QString created_path = quest.get_resource_element_path(resource_type, element_id);
    if (quest.exists(created_path)) {
      // Select the resource created.
      set_selected_path(created_path);

      // Open it.
      open_file_requested(quest, created_path);
    }
  }

  catch (const EditorException& ex) {
    ex.show_dialog();
  }

}

/**
 * @brief Slot called when the user wants to create a new directory
 * under the selected directory.
 *
 * The directory name will be prompted to the user.
 */
void QuestTreeView::new_directory_action_triggered() {

  QString path = get_selected_path();
  if (path.isEmpty()) {
    return;
  }

  try {
    bool ok = false;
    QString dir_name = QInputDialog::getText(
          this,
          tr("New folder"),
          tr("Folder name:"),
          QLineEdit::Normal,
          "",
          &ok);

    if (ok) {
      Quest::check_valid_file_name(dir_name);
      model->get_quest().create_dir(path, dir_name);

      // Select the directory created.
      QString dir_path = path + '/' + dir_name;
      set_selected_path(dir_path);
    }
  }

  catch (const EditorException& ex) {
    ex.show_dialog();
  }

}

/**
 * @brief Slot called when the user wants to create a new Lua script
 * under the selected directory.
 *
 * The file name will be prompted to the user.
 */
void QuestTreeView::new_script_action_triggered() {

  QString parent_path = get_selected_path();
  if (parent_path.isEmpty()) {
    return;
  }

  try {
    bool ok = false;
    QString file_name = QInputDialog::getText(
          this,
          tr("New Lua script"),
          tr("File name:"),
          QLineEdit::Normal,
          "",
          &ok);

    if (ok) {

      // Automatically add .lua extension if not present.
      if (!file_name.contains(".")) {
        file_name = file_name + ".lua";
      }
      Quest::check_valid_file_name(file_name);
      Quest& quest = model->get_quest();
      QString script_path = parent_path + '/' + file_name;
      quest.create_script(script_path);

      // Select the file created.
      set_selected_path(script_path);

      // Open it.
      open_file_requested(quest, script_path);
    }
  }
  catch (const EditorException& ex) {
    ex.show_dialog();
  }

}

/**
 * @brief Slot called when the user double-clicks or presses enter on a file.
 */
void QuestTreeView::default_action_triggered() {

  QString path = get_selected_path();
  if (path.isEmpty()) {
    return;
  }

  const Quest& quest = model->get_quest();
  ResourceType resource_type;
  QString element_id;
  if (quest.is_potential_resource_element(path, resource_type, element_id)) {

    if (quest.exists(path)) {
      if (resource_type == ResourceType::SOUND || resource_type == ResourceType::MUSIC) {
        play_action_triggered();
      }
      else {
        open_action_triggered();
      }
    }
  }
}

/**
 * @brief Slot called when the user wants to play the selected file.
 */
void QuestTreeView::play_action_triggered() {

  QString path = get_selected_path();
  if (path.isEmpty()) {
    return;
  }

  const Quest& quest = model->get_quest();
  ResourceType resource_type;
  QString element_id;
  if (quest.is_potential_resource_element(path, resource_type, element_id)) {
    if (resource_type == ResourceType::SOUND) {
      Sound::play_sound(model->get_quest(), element_id);
    }
    else if (resource_type == ResourceType::MUSIC) {
      Sound::play_music(model->get_quest(), element_id);
      // TODO or stop the music.
    }
  }

}

/**
 * @brief Slot called when the user wants to open the selected file.
 */
void QuestTreeView::open_action_triggered() {

  QString path = get_selected_path();
  if (path.isEmpty()) {
    return;
  }

  emit open_file_requested(model->get_quest(), path);
}

/**
 * @brief Slot called when the user wants to open the script of a map.
 */
void QuestTreeView::open_map_script_action_triggered() {

  QString path = get_selected_path();
  if (path.isEmpty()) {
    return;
  }

  Quest& quest = model->get_quest();
  ResourceType resource_type;
  QString element_id;
  if (!quest.is_resource_element(path, resource_type, element_id) ||
      resource_type != ResourceType::MAP) {
    return;
  }

  emit open_file_requested(quest, quest.get_map_script_path(element_id));
}

/**
 * @brief Slot called when the user wants to open the strings file of a
 * language.
 */
void QuestTreeView::open_language_strings_action_triggered() {

  QString path = get_selected_path();
  if (path.isEmpty()) {
    return;
  }

  Quest& quest = model->get_quest();
  ResourceType resource_type;
  QString element_id;
  if (!quest.is_resource_element(path, resource_type, element_id) ||
      resource_type != ResourceType::LANGUAGE) {
    return;
  }

  emit open_file_requested(quest, quest.get_strings_path(element_id));
}

/**
 * @brief Slot called when the user wants to rename the selected file or
 * directory.
 */
void QuestTreeView::rename_action_triggered() {

  emit rename_file_requested(model->get_quest(), get_selected_path());
}

/**
 * @brief Slot called when a file or directory of the quest was just renamed.
 * @param old_path File name before the change.
 * @param new_path File name after the change.
 */
void QuestTreeView::file_renamed(const QString& old_path,
                                 const QString& new_path) {

  if (get_selected_path() == old_path) {
    // Select the new file instead of the old one.
    set_selected_path(new_path);
  }
}

/**
 * @brief Slot called when the user wants to change the description of the
 * seelcted resource element.
 *
 * The new description will be prompted to the user.
 */
void QuestTreeView::change_description_action_triggered() {

  QString path = get_selected_path();
  if (path.isEmpty()) {
    return;
  }

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
  bool ok = false;
  QString new_description = QInputDialog::getText(
        this,
        tr("Change description"),
        tr("New description for %1 '%2':").arg(resource_friendly_type_name_for_id, element_id),
        QLineEdit::Normal,
        old_description,
        &ok);

  if (ok) {
    try {
      if (new_description.isEmpty()) {
        throw EditorException("Empty description");
      }
      resources.set_description(resource_type, element_id, new_description);
      resources.save();
    }
    catch (const EditorException& ex) {
      ex.show_dialog();
    }
  }
}

/**
 * @brief Slot called when the user wants to delete the selected file or
 * directory.
 *
 * Confirmation will be asked to the user.
 */
void QuestTreeView::delete_action_triggered() {

  const QString& path = get_selected_path();
  Quest& quest = model->get_quest();
  if (path == quest.get_data_path()) {
    // We don't want to delete the data directory.
    return;
  }
  ResourceType resource_type;
  if (quest.is_resource_path(path, resource_type)) {
    // Don't delete resource directories.
    return;
  }

  try {
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

      quest.delete_resource_element(resource_type, element_id);
    }

    else {
      // This is a regular file or directory.
      if (QFileInfo(path).isDir()) {
        QDir dir(path);
        bool empty_dir = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count() == 0;
        if (!empty_dir) {
          // TODO we could remove directories recursively,
          // but we have to take care of resources they contain.
          GuiTools::warning_dialog(tr("Folder is not empty"));
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
          quest.delete_dir(path);
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
        quest.delete_file(path);
      }
    }
  }
  catch (const EditorException& ex) {
    ex.show_dialog();
  }
}

}
