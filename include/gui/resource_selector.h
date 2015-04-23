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
#ifndef SOLARUSEDITOR_RESOURCE_SELECTOR_H
#define SOLARUSEDITOR_RESOURCE_SELECTOR_H

#include <QComboBox>
#include <solarus/ResourceType.h>

using ResourceType = Solarus::ResourceType;

class Quest;
class ResourceModel;
class QTreeView;

/**
 * @brief A combo box that allows to choose a resource element.
 *
 * Resource elements are presented in a tree view.
 */
class ResourceSelector : public QComboBox {
  Q_OBJECT

public:

  ResourceSelector(QWidget* parent);

  void set_quest(const Quest& quest);

  ResourceType get_resource_type() const;
  void set_resource_type(ResourceType resource_type);

  void add_special_value(const QString& id, const QString& text, int index);
  void remove_id(const QString& id);

  QString get_selected_id() const;
  void set_selected_id(const QString& element_id);

private:

  void rebuild();

  ResourceType resource_type;     /**< The type of resource to choose. */
  QTreeView* view;                /**< The tree view shown in the combobox. */
  ResourceModel* model;           /**< The underlying model. */

};

#endif
