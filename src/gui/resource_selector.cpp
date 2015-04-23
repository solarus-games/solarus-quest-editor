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
#include "gui/resource_selector.h"
#include "gui/resource_model.h"
#include <QHeaderView>
#include <QStandardItemModel>
#include <QTreeView>

/**
 * @brief Creates a resource selector.
 * @param parent The parent object or nullptr.
 */
ResourceSelector::ResourceSelector(QWidget* parent) :
  QComboBox(parent),
  resource_type(),
  view(nullptr),
  model(nullptr) {

}

/**
 * @brief Returns the type of resource shown in this combo box.
 * @return The type of resource.
 */
ResourceType ResourceSelector::get_resource_type() const {
  return resource_type;
}

/**
 * @brief Sets the type of resource shown in this combo box.
 * @param resource_type The type of resource.
 */
void ResourceSelector::set_resource_type(ResourceType resource_type) {

  if (resource_type == this->resource_type) {
    return;
  }

  this->resource_type = resource_type;

  if (model != nullptr) {
    model = new ResourceModel(model->get_quest(), resource_type);
    setModel(model);
  }
}

/**
 * @brief Sets the quest whose resources will be shown.
 * @param quest The quest.
 */
void ResourceSelector::set_quest(const Quest& quest) {

  model = new ResourceModel(quest, resource_type);
  setModel(model);

  view = new QTreeView(this);
  view->setModel(model);
  view->setEditTriggers(QTreeView::NoEditTriggers);
  view->setAlternatingRowColors(true);
  view->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  view->setHeaderHidden(true);

  setView(view);
}

/**
 * @brief Adds an item for a special value that is not an existing resource element.
 *
 * You can use this function to add a fake item like "None" or "Unchanged".
 *
 * @param id String to identify the item, replacing the resource element id.
 * @param text Text to show in the combo box for this item.
 * @param index Index where to insert the item.
 */
void ResourceSelector::add_special_value(
    const QString& id, const QString& text, int index) {

  if (model == nullptr) {
    return;
  }

  model->add_special_value(id, text, index);
}

/**
 * @brief Removes from the model the item with the specified id.
 * @param id Id of the resource to remove.
 */
void ResourceSelector::remove_id(const QString& id) {

  if (model == nullptr) {
    return;
  }

  model->remove_id(id);
}

/**
 * @brief Returns the id of the selected resource element.
 * @return The selected id.
 */
QString ResourceSelector::get_selected_id() const {

  return currentData(Qt::UserRole).toString();
}

/**
 * @brief Selects the specified resource element.
 * @param element_id Id of the element to select.
 */
void ResourceSelector::set_selected_id(const QString& element_id) {

  if (model == nullptr) {
    return;
  }

  const QModelIndex& index = model->get_element_index(element_id);
  if (!index.isValid()) {
    return;
  }

  // To make the label of the combobox show the correct value,
  // we need to call setCurrentIndex() or setCurrentText(),
  // but these functions only work with top-level elements of the hierarchy.
  // A workaround is to temporarily change the root of the tree.
  setRootModelIndex(index.parent());
  setCurrentIndex(index.row());  // Relative to the new root.
  setRootModelIndex(model->invisibleRootItem()->index());

  view->expand(index.parent());
}

/**
 * @brief Returns the id of the current tileset.
 *
 * This tileset is used for sprite resources.
 *
 * @return The current tileset id or an empty string.
 */
QString ResourceSelector::get_tileset_id() const {

  if (model == nullptr) {
    return QString();
  }

  return model->get_tileset_id();
}

/**
 * @brief Sets the id of the current tileset.
 *
 * This tileset is used for sprite resources.
 *
 * @param tileset_id The current tileset id or an empty string to unset it.
 */
void ResourceSelector::set_tileset_id(const QString& tileset_id) {

  if (model == nullptr) {
    return;
  }

  model->set_tileset_id(tileset_id);
}
