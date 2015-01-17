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
#include <QStandardItemModel>

/**
 * @brief Creates a resource selector.
 * @param parent The parent object or nullptr.
 */
ResourceSelector::ResourceSelector(QWidget* parent) :
  QComboBox(parent),
  resource_type(),
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
void ResourceSelector::set_quest(Quest& quest) {

  model = new ResourceModel(quest, resource_type);
  setModel(model);
}

/**
 * @brief Returns the id of the selected resource element.
 * @return The selected id.
 */
QString ResourceSelector::get_selected_id() const {

  // TODO
  return "";
}

/**
 * @brief Selects the specified resource element.
 * @param element_id Id of the element to select.
 */
void ResourceSelector::set_selected_id(const QString& /* element_id */) {
  // TODO
}
