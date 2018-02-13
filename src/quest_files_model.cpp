/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
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
#include "editor_exception.h"
#include "natural_comparator.h"
#include "quest.h"
#include "quest_files_model.h"
#include <QFileSystemModel>
#include <QItemSelection>

namespace SolarusEditor {

/**
 * @brief Creates a quest files model.
 * @param parent Path of the quest to represent.
 */
QuestFilesModel::QuestFilesModel(Quest& quest):
  QSortFilterProxyModel(nullptr),
  quest(quest),
  source_model(new QFileSystemModel) {

  // Watch changes on the filesystem.
  source_model->setRootPath(quest.get_data_path());  // Only watch changes in the data directory.
  source_model->setReadOnly(false);
  setSourceModel(source_model);

  // Watch changes in resources.
  connect(&quest.get_resources(), SIGNAL(element_added(ResourceType, QString, QString)),
          this, SLOT(resource_element_added(ResourceType, QString, QString)));
  connect(&quest.get_resources(), SIGNAL(element_removed(ResourceType, QString)),
          this, SLOT(resource_element_removed(ResourceType, QString)));
  connect(&quest.get_resources(), SIGNAL(element_renamed(ResourceType, QString, QString)),
          this, SLOT(resource_element_renamed(ResourceType, QString, QString)));
  connect(&quest.get_resources(), SIGNAL(element_description_changed(ResourceType, QString, QString)),
          this, SLOT(resource_element_description_changed(ResourceType, QString, QString)));

  // This model adds extra items for files missing on the filesystem.
  // To ensure we have an extra item if and only if the file is missing,
  // we need to watch files creations and destructions.
  connect(source_model, SIGNAL(rowsInserted(QModelIndex, int, int)),
          SLOT(source_model_rows_inserted(QModelIndex, int, int)));
  connect(source_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
          SLOT(source_model_rows_about_to_be_removed(QModelIndex, int, int)));
}

/**
 * @brief Returns the quest represented by this model.
 * @return The quest.
 */
Quest& QuestFilesModel::get_quest() {
  return quest;
}

/**
 * @brief Returns the index of the root item of the quest.
 * @return The root index.
 */
QModelIndex QuestFilesModel::get_quest_root_index() const {

  return mapFromSource(source_model->index(quest.get_root_path()));
}

/**
 * @brief Returns the number of columns in the model.
 *
 * Reimplemented to set custom columns.
 *
 * @param parent Parent index.
 * @return The number of columns
 */
int QuestFilesModel::columnCount(const QModelIndex& parent) const {

  Q_UNUSED(parent);

  // File, Description, Type.
  return NUM_COLUMNS;
}

/**
 * @brief Returns the number of rows under a parent item.
 *
 * Reimplemented to add rows with declared resource elements that are missing
 * on the filesystem.
 *
 * @param parent Parent index.
 * @return The number of rows under this parent.
 */
int QuestFilesModel::rowCount(const QModelIndex& parent) const {

  // Get the number of rows from the filesytem (filtered)
  // and add extra rows for resource element whose file is missing.
  int existing_files_count = QSortFilterProxyModel::rowCount(parent);
  int missing_files_count = get_num_extra_paths(parent);

  return existing_files_count + missing_files_count;
}

/**
 * @brief Returns the index of an item.
 *
 * Reimplemented from QSortFilterProxyModel to create custom indexes
 * for items that are not in the source modelk
 * Such items represent files expected by the quest, shown in the model but
 * that are missing on the filesystem.
 *
 * @param row Row of the item.
 * @param column Column of the item.
 * @param parent Parent of the item.
 * @return The corresponding index. Returns an invalid index if there is no
 * such item.
 */
QModelIndex QuestFilesModel::index(int row, int column, const QModelIndex& parent) const {

  if (row < 0 || column < 0) {
      return QModelIndex();
  }

  QModelIndex index_mapped_from_source = QSortFilterProxyModel::index(row, column, parent);
  if (index_mapped_from_source.isValid()) {
    return index_mapped_from_source;
  }

  // Item does not exist in the source model: this is an extra item of
  // QuestFilesModel. Determine its file path.
  int num_existing = QSortFilterProxyModel::rowCount(parent);
  int index_in_extra = row - num_existing;
  ExtraPaths* extra_paths = get_extra_paths(parent);
  if (extra_paths == nullptr) {
    // The parent is not an item expecting extra children.
    return QModelIndex();
  }

  if (index_in_extra < 0 || index_in_extra >= extra_paths->element_ids.size()) {
    // The row parameter is out of range.
    return QModelIndex();
  }

  // Create a custom index for this path.
  QString* extra_path_ptr = extra_paths->paths.at(index_in_extra)[column];
  return createIndex(row, column, extra_path_ptr);
}

/**
 * @brief Returns the parent of the model item with the given index.
 *
 * Reimplemented from QSortFilterProxyModel to support extra indexes
 * that do not exist in the source model.
 *
 * @param index Index to get the parent of.
 * @return The parent index, or an invalid index if the item has no parent.
 */
QModelIndex QuestFilesModel::parent(const QModelIndex& index) const {

  QString path;
  if (is_extra_path(index, path)) {
    // Index that does not exist in the source model.
    QDir parent_dir(path);
    if (!parent_dir.cdUp()) {
      return QModelIndex();
    }
    return get_file_index(parent_dir.path());
  }

  // Regular QSortFilterProxyModel index.
  return QSortFilterProxyModel::parent(index);
}

/**
 * @brief Returns the sibling at the given position of an item.
 *
 * Reimplemented from QSortFilterProxyModel to support extra indexes
 * that do not exist in the source model.
 *
 * @param row Row of the index to get.
 * @param column Column of the index to get.
 * @param idx An existing index with the same parent as the wanted result.
 * @return The sibling.
 */
QModelIndex QuestFilesModel::sibling(int row, int column, const QModelIndex& idx) const {

  // Reimplementation needed because QSortFilterProxyModel::sibling()
  // assumes that indexes can always be mapped to the source model.
  return index(row, column, parent(idx));
}

/**
 * @brief Returns whether an item has children.
 * @param parent The item to test.
 * @return @c true if this item has children.
 */
bool QuestFilesModel::hasChildren(const QModelIndex& parent) const {

  QString file_path = get_file_path(parent);
  ResourceType resource_type;

  if (QSortFilterProxyModel::hasChildren(parent))  {
    // This is a non-empty directory.
    return true;
  }

  // The directory is empty, but resources might be declared there and missing.
  if (!quest.is_resource_path(file_path, resource_type) &&
      !quest.is_in_resource_path(file_path, resource_type)) {
    // This is not a resource directory, nothing special was declared here.
    return false;
  }

  // This is a resource directory and it is empty.
  // To see if some resources are declared there, ask the quest
  // (this is faster than calling get_missing_resource_elements()).
  if (quest.has_resource_element(file_path, resource_type)) {
    return true;
  }

  // The directory is empty and no resource is declared in it.
  return false;
}

/**
 * @brief Converts an index of this model to an index in the source model.
 *
 * Reimplemented from QSortFilterProxyModel to peacefully return an invalid
 * source index if the item does not exist in the source model.
 *
 * @param proxy_index An index in this model.
 * @return The corresponding source index, or an invalid index if this is an
 * extra index that does not exist in the source model.
 */
QModelIndex QuestFilesModel::mapToSource(const QModelIndex& proxy_index) const {

  QString path;
  if (is_extra_path(proxy_index, path)) {
    // This item does not exist in the source model
    // (it was added by us).
    return QModelIndex();
  }

  return QSortFilterProxyModel::mapToSource(proxy_index);
}

/**
 * @brief Converts a selection of this model to a selection in the source model.
 *
 * Reimplemented from QSortFilterProxyModel to peacefully handle items that do
 * not exist in the source model.
 *
 * @param proxy_selection A selection in this model.
 * @return The corresponding source selection.
 */
QItemSelection QuestFilesModel::mapSelectionToSource(const QItemSelection& proxy_selection) const {

  QItemSelection source_selection;

  const QModelIndexList& indexes = proxy_selection.indexes();
  for (const QModelIndex& index : indexes) {
    const QModelIndex& source_index = mapToSource(index);
    if (!index.isValid()) {
      // Selected item that does not exist in the source model.
      continue;
    }
    source_selection.append(QItemSelectionRange(source_index));
  }

  return source_selection;
}

/**
 * @brief Returns the flags of an item.
 * @param index An item index.
 * @return The item flags.
 */
Qt::ItemFlags QuestFilesModel::flags(const QModelIndex& index) const {

  QString file_path = get_file_path(index);
  ResourceType resource_type;
  QString element_id;
  Qt::ItemFlags flags =  Qt::ItemIsSelectable | Qt::ItemIsEnabled;

  switch (index.column()) {

  case FILE_COLUMN:  // File name.

    if (quest.is_resource_element(file_path, resource_type, element_id)) {
      // Resource elements never has children,
      // except languages that are actually directories on the filesystem.
      if (resource_type != ResourceType::LANGUAGE) {
        flags |= Qt::ItemNeverHasChildren;
      }
    }
    return flags;

  case DESCRIPTION_COLUMN:  // Resource description.

    if (quest.is_resource_element(file_path, resource_type, element_id)) {
      // The description column of a resource element can be modified.
      return flags | Qt::ItemIsEditable;
    }
    return flags;
  }

  return flags;
}

/**
 * @brief Returns the header info.
 * @param section Column or row number.
 * @param orientation Horizontal or vertical header orientation.
 * @param role The data under this role will be returned.
 * @return The header data.
 */
QVariant QuestFilesModel::headerData(int section, Qt::Orientation orientation, int role) const {

  switch (role) {

  case Qt::DisplayRole:
    // Text of each header.
    switch (section) {

    case FILE_COLUMN:
      return tr("Resource");

    case DESCRIPTION_COLUMN:
      return tr("Description");

    case TYPE_COLUMN:
      return tr("Type");
    }
    return QVariant();
  }

  // For other roles, rely on the standard header settings.
  return QSortFilterProxyModel::headerData(section, orientation, role);
}

/**
 * @brief Returns the data of an item for a given role.
 * @param index Index of the item to get.
 * @param role The data under this role will be returned.
 * @return The data.
 */
QVariant QuestFilesModel::data(const QModelIndex& index, int role) const {

  const QuestResources& resources = quest.get_resources();
  ResourceType resource_type;
  QString element_id;

  QString path = get_file_path(index);
  QString file_name = QFileInfo(path).fileName();

  switch (role) {

  case Qt::DisplayRole:
    // Text of each file item.

    switch (index.column()) {

    case FILE_COLUMN:  // File name.
      if (is_quest_data_index(index)) {
        // Data directory: show the quest name instead of "data".
        return quest.get_name();
      }

      if (quest.is_resource_element(path, resource_type, element_id)) {
        // A resource element: show its id (remove the extension).
        return QFileInfo(path).completeBaseName();
      }
      return file_name;

    case DESCRIPTION_COLUMN:  // Resource element description.

      if (!quest.is_resource_element(path, resource_type, element_id)) {
        return QVariant();
      }
      return resources.get_description(resource_type, element_id);

    case TYPE_COLUMN:  // Type.
      if (is_quest_data_index(index)) {
        // Quest data directory (top-level item).
        return tr("Quest");
      }

      if (path == quest.get_main_script_path()) {
        // main.lua
        return tr("Main Lua script");
      }

      if (quest.is_resource_path(path, resource_type)) {
        // A resource element folder.
        return resources.get_directory_friendly_name(resource_type);
      }

      if (quest.is_resource_element(path, resource_type, element_id)) {
        // A declared resource element.
        return resources.get_friendly_name(resource_type);
      }

      if (quest.is_script(path)) {
        // A Lua script.
        return tr("Lua script");
      }

      if (quest.is_image(path)) {
        // A PNG image.
        return tr("Image");
      }

      if (quest.is_data_file(path)) {
        // A .dat file.
        return tr("Data file");
      }

      // Not a file managed by Solarus.
      return QVariant();
    }

  case Qt::EditRole:
    // Editable file name.
    switch (index.column()) {

    case FILE_COLUMN:  // File name.
      return file_name;

    case DESCRIPTION_COLUMN:
      // The resource element description can be edited.
      if (!quest.is_resource_element(path, resource_type, element_id)) {
        return QVariant();
      }
      return resources.get_description(resource_type, element_id);
    }

  case Qt::DecorationRole:
    // Icon.
    if (index.column() == FILE_COLUMN) {
      return get_quest_file_icon(index);
    }
    return QVariant();  // No icon in other columns.

  case Qt::ToolTipRole:
    // Tooltip.
    if (index.column() == FILE_COLUMN) {
      return get_quest_file_tooltip(index);
    }
    return QVariant();  // No tooltip in other columns.

  case Qt::TextAlignmentRole:
    // Remove the alignment done by QFileSystemModel.
    return Qt::AlignLeft;
  }

  QString extra_path;
  if (is_extra_path(index, extra_path)) {
    // File expected by the quest but missing on the filesystem.
    return QVariant();
  }

  // For other roles, rely on standard settings.
  return QSortFilterProxyModel::data(index, role);
}

/**
 * @brief Sets the data of an item for a given role.
 * @param index Index of the item to set.
 * @param value The new value to set.
 * @param role The role to change.
 * @parem @c true in case of success.
 */
bool QuestFilesModel::setData(
    const QModelIndex& index, const QVariant& value, int role) {

  if (index.column() != DESCRIPTION_COLUMN) {
    // Only the description column is editable.
    return false;
  }

  if (role != Qt::EditRole) {
    return false;
  }

  QString file_path = get_file_path(index);
  ResourceType resource_type;
  QString element_id;
  if (!quest.is_resource_element(file_path, resource_type, element_id)) {
    return false;
  }

  try {
    quest.get_resources().set_description(resource_type, element_id, value.toString());
    quest.get_resources().save();
    emit dataChanged(index, index);
    return true;
  }
  catch (const EditorException& ex) {
    ex.print_message();
    return false;
  }
}

/**
 * @brief Returns an appropriate icon for the specified quest file.
 * @param index Index of a file item in the model.
 * @return An appropriate icon name to represent this file.
 */
QIcon QuestFilesModel::get_quest_file_icon(const QModelIndex& index) const {

  QString icon_file_name;  // Relative to the icons directory.
  QString file_path = get_file_path(index);
  ResourceType resource_type;
  QString element_id;

  // Quest data directory.
  if (is_quest_data_index(index)) {
    icon_file_name = "icon_solarus.png";
  }

  // Resource element (possibly a directory for languages).
  else if (quest.is_resource_element(file_path, resource_type, element_id)) {

    QString resource_type_name = quest.get_resources().get_lua_name(resource_type);
    if (quest.exists(quest.get_resource_element_path(resource_type, element_id))) {
      // Resource declared and present on the filesystem.
      icon_file_name = "icon_resource_" + resource_type_name + ".png";
    }
    else {
      // Resource declared but whose file is missing.
      icon_file_name = "icon_resource_" + resource_type_name + "_missing.png";
    }
  }

  // Dialogs file (under a language resource element).
  else if (quest.is_dialogs_file(file_path, element_id)) {
    icon_file_name = "icon_dialogs.png";
  }

  // Strings file (under a language resource element).
  else if (quest.is_strings_file(file_path, element_id)) {
    icon_file_name = "icon_strings.png";
  }

  // Directory icon.
  else if (quest.is_dir(file_path)) {

    if (quest.is_resource_path(file_path, resource_type)) {
      QString resource_type_name = quest.get_resources().get_lua_name(resource_type);
      icon_file_name = "icon_folder_open_" + resource_type_name + ".png";
    }
    else {
      icon_file_name = "icon_folder_open.png";
    }
  }

  // Lua script icon.
  else if (quest.is_script(file_path)) {
    icon_file_name = "icon_script.png";
  }

  // Image icon.
  else if (quest.is_image(file_path)) {
    icon_file_name = "icon_image.png";
  }

  // Generic icon for a file not known by the quest.
  else {
    icon_file_name = "icon_file_unknown.png";
  }

  if (icon_file_name.isEmpty()) {
    return QIcon();
  }

  return QIcon(":/images/" + icon_file_name);
}

/**
 * @brief Returns an appropriate tooltip for the specified quest file.
 * @param index Index of a file item in the model.
 * @return An appropriate tooltip for this file item.
 */
QString QuestFilesModel::get_quest_file_tooltip(const QModelIndex& index) const {

  QString path = get_file_path(index);
  ResourceType resource_type;
  QString element_id;

  // Show a tooltip for resource elements because their item text is different
  // from the physical file name.
  if (quest.is_potential_resource_element(path, resource_type, element_id)) {

    QString file_name = QFileInfo(path).fileName();
    if (quest.get_resources().exists(resource_type, element_id)) {
      // Declared in the resource list.
      if (quest.exists(quest.get_resource_element_path(resource_type, element_id))) {
        // Declared in the resource list and existing on the filesystem.
        return file_name;
      }
      else {
        // Declared in the resource list and not found on the filesystem.
        return tr("%1 (file not found)").arg(file_name);
      }
    }
    else {
      // Found on the filesystem but not declared in the resource list.
      return tr("%1 (not in the quest)").arg(file_name);
    }
  }

  return "";
}

/**
 * @brief Compares two items for sorting purposes.
 * @param left An item index in the source model.
 * @param right Another item index in the source model.
 * @return @c true if the value of the first item is less than the second one.
 */
bool QuestFilesModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {

  // Directories are before regular files.
  if (source_model->isDir(left) &&
      !source_model->isDir(right)) {
    return true;
  }

  if (!source_model->isDir(left) &&
      source_model->isDir(right)) {
    return false;
  }

  const QString& left_string = source_model->data(left, Qt::DisplayRole).toString();
  const QString& right_string = source_model->data(right, Qt::DisplayRole).toString();
  if (!left_string.isEmpty() && !right_string.isEmpty()) {
    // If we are comparing strings displayed to the user, use the natural
    // comparator so that "tileset_2" is before "tileset_10".
    return NaturalComparator()(left_string, right_string);
  }

  return QSortFilterProxyModel::lessThan(left, right);
}

/**
 * @brief Returns whether a source row should be included in the model.
 * @param source_row A row in the source model.
 * @param source_parent Parent index of the row in the source model.
 * @return @c true to keep the row, @c false to filter it out.
 */
bool QuestFilesModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {

  QModelIndex source_index = source_model->index(source_row, FILE_COLUMN, source_parent);
  QString file_name = source_model->fileName(source_index);
  QString file_path = source_model->filePath(source_index);

  if (source_model->index(source_model->rootPath()).parent() == source_parent) {
    // This is a top-level item: only keep the quest data directory.
    if (file_path == quest.get_data_path()) {
      return true;
    }
    return false;
  }

  if (source_model->isDir(source_index)) {
    // Keep all other directories.
    return true;
  }

  const QString lua_extension = ".lua";
  if (file_name.endsWith(lua_extension, Qt::CaseInsensitive)) {
    // Keep all .lua scripts except map scripts.
    QString file_path_dat = file_path.toLower().replace(file_path.lastIndexOf(lua_extension), lua_extension.size(), ".dat");
    ResourceType resource_type;
    QString element_id;
    if (quest.is_resource_element(file_path_dat, resource_type, element_id) &&
        resource_type == ResourceType::MAP) {
      return false;
    }

    return true;
  }

  // Keep resources, and also files that could be resources
  // but are not declared in the resource list yet.
  ResourceType resource_type;
  QString element_id;
  if (quest.is_potential_resource_element(file_path, resource_type, element_id)) {
    return true;
  }

  // Keep .dat files.
  if (quest.is_data_file(file_path)) {

    // Except quest.dat and project_db.dat.
    if (quest.is_properties_path(file_path)) {
      // Quest properties file quest.dat.
      return false;
    }

    if (quest.is_resource_list_path(file_path)) {
      // Quest resource list project_db.dat.
      return false;
    }

    return true;
  }

  // Keep .png files.
  if (file_name.endsWith(".png", Qt::CaseInsensitive)) {

    // Except tileset ones.
    QString tileset_id;
    if (quest.is_tileset_tiles_file(file_path, tileset_id) ||
        quest.is_tileset_entities_file(file_path, tileset_id)) {
      return false;
    }

    return true;
  }

  // File not known by the quest.
  return false;
}

/**
 * @brief Returns whether a source index is the index of the quest data directory.
 *
 * This function returns @c true for all columns of the data directory row.
 *
 * @param index An index in the model.
 * @return @c true if this is the quest data directory.
 */
bool QuestFilesModel::is_quest_data_index(const QModelIndex& index) const {

  return get_file_path(index) == quest.get_data_path();
}

/**
 * @brief Returns whether a source index is the index of a directory that
 * exists on the filesystem.
 *
 * This is an internal utility function.
 * Note that it returns @c true for all actual directories including
 * language resource elements.
 *
 * @param index An index in the model.
 * @return @c true if this is an existing directory.
 */
bool QuestFilesModel::is_dir_on_filesystem(const QModelIndex& index) const {

  QString extra_path;
  if (is_extra_path(index, extra_path)) {
    // This item does not exist in the filesystem (it was added by us).
    // So it is clearly not an existing directory.
    return false;
  }

  // For performance, ask the filesystem model rather than the quest
  // since the filesystem model already has the information in cache.
  const QModelIndex& source_index = mapToSource(index);
  return source_model->isDir(source_index);
}

/**
 * @brief Returns whether an index is an extra path that does not exist in the
 * source model.
 * @param[in] index The index to test.
 * @param[out] path The extra path corresponding to this index if any.
 * @return @c true if this is an extra path.
 */
bool QuestFilesModel::is_extra_path(
    const QModelIndex& index, QString& extra_path) const {

  void* internal_ptr = index.internalPointer();
  auto it = all_extra_paths.find(static_cast<QString*>(internal_ptr));
  if (it == all_extra_paths.end()) {
    // Index managed by QSortFilterProxyModel.
    return false;
  }

  // Index managed by us.
  extra_path = **it;
  return true;
}

/**
 * @brief Returns the path of the file at the specified index.
 *
 * Only the row of the index is considered, so the result is the same for all
 * columns.
 *
 * @param index Index of an item in this model.
 * @return The corresponding path.
 */
QString QuestFilesModel::get_file_path(const QModelIndex& index) const {

  if (!index.isValid()) {
    return "";
  }

  QString extra_path;
  if (is_extra_path(index, extra_path)) {
    // The item is a declared resource element whose file is missing
    // (see how rowCount() adds rows for missing elements).
    return extra_path;
  }

  // The item is a file that exists on the filesystem.
  QModelIndex source_index = mapToSource(index);
  QModelIndex file_source_index = source_model->index(
        source_index.row(), FILE_COLUMN, source_index.parent());
  return source_model->filePath(file_source_index);
}

/**
 * @brief Returns the index of the specified path.
 *
 * The column of the returned index is always FILE_COLUMN.
 *
 * @param path Path of a file.
 * @return The corresponding index. An invalid index is returned if the path
 * does not exist in the model.
 */
QModelIndex QuestFilesModel::get_file_index(const QString& path) const {

  const QModelIndex& index_from_source = mapFromSource(source_model->index(path, 0));
  if (index_from_source.isValid()) {
    // The path exists in the source model and therefore on the filesystem.
    return index_from_source;
  }

  // The file does not exist.
  // It can be a resource element added to the tree but whose file is missing.
  // Find its place in the tree to know the row.
  QDir parent_dir(path);
  if (!parent_dir.cdUp()) {
    return QModelIndex();
  }
  const QModelIndex& parent = get_file_index(parent_dir.path());

  const ExtraPaths* extra_paths = get_extra_paths(parent);
  if (extra_paths == nullptr) {
    // Path not in the model.
    return QModelIndex();
  }

  const auto& it = extra_paths->path_indexes.find(path);
  if (it == extra_paths->path_indexes.end()) {
    // Path not in the model.
    return QModelIndex();
  }
  const int index_in_extra = it.value();
  const int row = QSortFilterProxyModel::rowCount(parent) + index_in_extra;
  return index(row, 0, parent);
}

/**
 * @brief Returns the number of resource elements that are declared under an item
 * but whose files are missing on the filesystem.
 * @param[in] parent An index in the model. If this is a directory, resources
 * directly under this directory are counted. Otherwise, returns 0.
 * @return The number of resource elements declared but missing under the item.
 */
int QuestFilesModel::get_num_extra_paths(const QModelIndex& parent) const {

  const ExtraPaths* extra_paths = get_extra_paths(parent);
  if (extra_paths == nullptr) {
    return 0;
  }
  return extra_paths->paths.size();
}

/**
 * @brief Returns the resource elements that are declared under an item
 * but whose files are missing on the filesystem.
 * @param[in] parent An index in the model. If this is a directory, resources
 * directly under this directory are checked. Otherwise, returns an empty list.
 * @return The ids of resource elements whose file is missing under the item.
 * Returns nullptr if this is not a directory or not a directory expecting resources.
 */
QuestFilesModel::ExtraPaths* QuestFilesModel::get_extra_paths(const QModelIndex& parent) const {

  const QString& parent_path = get_file_path(parent);
  auto it = extra_paths_by_dir.find(parent_path);
  if (it == extra_paths_by_dir.end()) {
    // The info is not computed yet.
    compute_extra_paths(parent);
    it = extra_paths_by_dir.find(parent_path);
    if (it == extra_paths_by_dir.end()) {
      // Not a directory or not a directory expecting resources.
      return nullptr;
    }
  }

  ExtraPaths* extra_paths = &it.value();
  return extra_paths;
}

/**
 * @brief Determines the ids of resource elements that are declared under an item
 * but whose files are missing on the filesystem.
 * @param[in] parent An index in the model. If this is a directory, resources
 * directly under this directory are checked. Otherwise, nothing is done.
 * @param[out] resource_type The type of resource that can exist under the item.
 */
void QuestFilesModel::compute_extra_paths(const QModelIndex& parent) const {

  QString parent_path = get_file_path(parent);

  // First clear the cache, we are about to rebuild it.
  extra_paths_by_dir.remove(parent_path);

  if (!is_dir_on_filesystem(parent)) {
    // Parent is not a directory: nothing more to do.
    return;
  }

  ResourceType resource_type;
  QString element_id;
  if (quest.is_resource_element(parent_path, resource_type, element_id)) {
    // This is a leaf item. In particular, we ignore the subtree of languages.
    return;
  }

  if (!quest.is_resource_path(parent_path, resource_type) &&
      !quest.is_in_resource_path(parent_path, resource_type)) {
    // Parent is not a resource directory: we will not find resources there.
    return;
  }

  ExtraPaths& extra_paths = extra_paths_by_dir[parent_path];

  // Get all declared elements of this resource type that are directly in
  // the directory.
  const QStringList& element_ids = quest.get_resources().get_elements(resource_type);
  for (const QString& element_id : element_ids) {
    QString current_path = quest.get_resource_element_path(resource_type, element_id);
    if (!current_path.startsWith(parent_path)) {
      // The current element is not under our directory.
      continue;
    }

    QString current_path_from_parent = current_path.right(current_path.size() - parent_path.size() - 1);
    if (current_path_from_parent.indexOf('/') != -1) {
      // The current element is not a direct child of our directory.
      continue;
    }

    // The current resource element is declared in this directory.
    // Check that its file exists.
    // Note: we could check the existence of the file faster by asking the source model,
    // but we want this computation to work even from rowsAboutToBeRemoved(), that is,
    // when model rows of files just deleted still exist in the source model.
    if (!quest.exists(quest.get_resource_element_path(resource_type, element_id))) {
      // This is an extra element. Insert it in the cache.
      ExtraPathColumnPtrs columns;
      for (int j = 0; j < NUM_COLUMNS; ++j) {
        QString* path_internal_ptr = new QString(current_path);
        columns[j] = path_internal_ptr;
        all_extra_paths.insert(path_internal_ptr);
      }
      extra_paths.paths.append(columns);
      extra_paths.path_indexes[current_path] = extra_paths.paths.size() - 1;
      extra_paths.element_ids.append(element_id);
    }
  }
}

/**
 * @brief Slot called a resource element is added to the resource list.
 *
 * The corresponding files may or may not already exist on the filesystem.
 *
 * @param resource_type A type of resource.
 * @param element_id Id of the element that was added.
 * @param description The element description.
 */
void QuestFilesModel::resource_element_added(
    ResourceType resource_type, const QString& element_id, const QString& description) {

  Q_UNUSED(description);

  // If the file already exists, it automatically appears in the tree
  // thanks to QFileSystemWatcher.
  // Otherwise, we insert it as an extra path.

  const QString& path = quest.get_resource_element_path(resource_type, element_id);
  if (quest.exists(path)) {
    return;
  }

  QDir parent_dir(path);
  if (!parent_dir.cdUp()) {
    // The parent does not exist: nothing to do.
    return;
  }
  const QModelIndex& parent = get_file_index(parent_dir.path());
  insert_extra_path(parent, path);
}

/**
 * @brief Slot called when a resource element is removed from the resource list.
 *
 * The corresponding files may or may not continue to exist on the filesystem.
 *
 * @param resource_type A type of resource.
 * @param element_id Id of the element that was removed.
 */
void QuestFilesModel::resource_element_removed(
    ResourceType resource_type, const QString& element_id) {

  // If the file existed, it automatically disappears from
  // the tree thanks to QFileSystemWatcher.
  // Otherwise, since it was it in the tree anyway, we need to notify people
  // that there a row was removed.

  QString path = quest.get_resource_element_path(resource_type, element_id);

  QDir parent_dir(path);
  if (!parent_dir.cdUp()) {
    // The parent no longer exists either: nothing to do.
    return;
  }
  const QModelIndex& parent = get_file_index(parent_dir.path());

  // See if this was an extra path (not existing in the source model).
  remove_extra_path(parent, path);
}

/**
 * @brief Slot called when the id of a resource element changes in the resource list.
 *
 * The corresponding files may or may not exist with the old or new file name
 * on the filesystem.
 *
 * @param resource_type A type of resource.
 * @param old_id Id of the element before the change.
 * @param new_id New id of the element.
 */
void QuestFilesModel::resource_element_renamed(
    ResourceType resource_type, const QString& old_id, const QString& new_id) {

  resource_element_removed(resource_type, old_id);
  resource_element_added(resource_type, new_id,
                         quest.get_resources().get_description(resource_type, new_id));
}

/**
 * @brief Slot called when the description of a resource element changes.
 * @param resource_type A type of resource.
 * @param element_id Id of the element whose description has changed.
 * @param description The new description.
 */
void QuestFilesModel::resource_element_description_changed(
    ResourceType resource_type, const QString& element_id, const QString& description) {

  Q_UNUSED(description);
  QModelIndex index =
      get_file_index(quest.get_resource_element_path(resource_type, element_id));
  index = sibling(index.row(), DESCRIPTION_COLUMN, index);
  emit dataChanged(index, index);
}

/**
 * @brief Slot called when a file (or more) appears in the source model.
 *
 * If the file there as an extra path in this model, the extra path is removed
 * now that the physical file exists.
 *
 * @param source_parent Parent source index of the added items.
 * @param first First row of the added source items.
 * @param last Last row of the added source items.
 */
void QuestFilesModel::source_model_rows_inserted(const QModelIndex& source_parent, int first, int last) {

  const QModelIndex& parent = mapFromSource(source_parent);
  if (!parent.isValid()) {
    // The parent item was not created yet in this model
    // or was filtered out.
    return;
  }

  const ExtraPaths* extra_paths = get_extra_paths(parent);
  if (extra_paths == nullptr || extra_paths->paths.isEmpty()) {
    // There are currently no extra paths here.
    return;
  }

  for (int source_row = first; source_row <= last; ++source_row) {
    const QModelIndex& source_index = source_model->index(source_row, 0, source_parent);
    const QString& path = source_model->filePath(source_index);
    remove_extra_path(parent, path);
  }
}

/**
 * @brief Slot called when a file (or more) is disappearing in the source model.
 *
 * If the file was a resource element, is becomes missing so an extra path is
 * added by this model to replace it and still show an item.
 *
 * @param source_parent Parent source index of the removed items.
 * @param first First row of the removed source items.
 * @param last Last row of the removed source items.
 */
void QuestFilesModel::source_model_rows_about_to_be_removed(const QModelIndex& source_parent, int first, int last) {

  const QModelIndex& parent = mapFromSource(source_parent);
  if (!parent.isValid()) {
    // The parent item was filtered out.
    return;
  }

  for (int source_row = first; source_row <= last; ++source_row) {
    const QModelIndex& source_index = source_model->index(source_row, 0, source_parent);
    const QString& path = source_model->filePath(source_index);
    insert_extra_path(parent, path);
  }
}

/**
 * @brief If the specified path does not exist as an extra path yet, inserts it in the model.
 * @param parent Parent directory where to insert the path.
 * @param path The path to insert.
 */
void QuestFilesModel::insert_extra_path(const QModelIndex& parent, const QString& path) {

  ResourceType resource_type;
  QString element_id;
  if (!quest.is_resource_element(path, resource_type, element_id)) {
    // Only resource elements can create extra paths.
    return;
  }

  ExtraPaths* extra_paths = get_extra_paths(parent);
  if (extra_paths == nullptr) {
    // First extra path of this directory: create the extra paths cache.
    const QString& parent_path = get_file_path(parent);
    extra_paths_by_dir[parent_path] = ExtraPaths();
    extra_paths = get_extra_paths(parent);
    if (extra_paths == nullptr) {
      // Something is wrong.
      return;
    }
  }

  auto it = extra_paths->path_indexes.find(path);
  if (it != extra_paths->path_indexes.end()) {
    // There is already an extra path for this item.
    return;
  }

  // Determine the index where to insert the new item.
  int index_in_extra = 0;
  const int regular_row_count = QSortFilterProxyModel::rowCount(parent);
  for (index_in_extra = 0; index_in_extra < extra_paths->paths.size(); ++index_in_extra) {
    const int current_row = regular_row_count + index_in_extra;
    const QModelIndex& current_index = index(current_row, 0, parent);
    const QString& current_path = get_file_path(current_index);
    if (current_path > path) {
      break;
    }
  }
  const int row = regular_row_count + index_in_extra;

  beginInsertRows(parent, row, row);

  ExtraPathColumnPtrs columns;
  for (int j = 0; j < NUM_COLUMNS; ++j) {
    QString* path_internal_ptr = new QString(path);
    columns[j] = path_internal_ptr;
    all_extra_paths.insert(path_internal_ptr);
  }
  extra_paths->element_ids.insert(index_in_extra, element_id);
  extra_paths->paths.insert(index_in_extra, columns);

  // Rebuild the index map because indexes get shifted.
  extra_paths->rebuild_index_cache();

  endInsertRows();
}

/**
 * @brief If the specified path exists as an extra path, removes it from the model.
 * @param parent Parent directory where to remove the path.
 * @param path The path to remove.
 */
void QuestFilesModel::remove_extra_path(const QModelIndex& parent, const QString& path) {

  ExtraPaths* extra_paths = get_extra_paths(parent);
  if (extra_paths == nullptr || extra_paths->paths.isEmpty()) {
    // No extra path here.
    return;
  }
  const auto& it = extra_paths->path_indexes.find(path);
  if (it == extra_paths->path_indexes.end()) {
    // There is already no extra path for this item.
    return;
  }

  // Remove the extra path.
  const int index_in_extra = it.value();
  const int regular_row_count = QSortFilterProxyModel::rowCount(parent);
  const int row = regular_row_count + index_in_extra;

  beginRemoveRows(parent, row, row);

  for (QString* path_internal_ptr : extra_paths->paths.at(index_in_extra)) {
    all_extra_paths.remove(path_internal_ptr);
  }

  extra_paths->paths.removeAt(index_in_extra);
  extra_paths->element_ids.removeAt(index_in_extra);

  // Rebuild the index map because indexes get shifted.
  extra_paths->rebuild_index_cache();
  endRemoveRows();
}

/**
 * @brief Destroys an extra paths objects.
 *
 * TODO use unique_ptr to get rid of this destructor.
 */
QuestFilesModel::ExtraPaths::~ExtraPaths() {

  for (ExtraPathColumnPtrs& ptrs : paths) {
    for (QString* path_internal_ptr : ptrs) {
      delete path_internal_ptr;
    }
  }
}

/**
 * @brief Rebuilds the extra path to indexes internal mapping.
 *
 * Call this function when extra path indexes are changed.
 */
void QuestFilesModel::ExtraPaths::rebuild_index_cache() {

  path_indexes.clear();
  int i = 0;
  for (const ExtraPathColumnPtrs& columns : paths) {
    const QString& current_path = *columns[0];
    path_indexes.insert(current_path, i);
    ++i;
  }
}

}
