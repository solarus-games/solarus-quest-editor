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
#include "gui/sprite_tree_view.h"
#include "sprite_model.h"
#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>

/**
 * @brief Creates an empty sprite tree view.
 * @param parent The parent object or nullptr.
 */
SpriteTreeView::SpriteTreeView(QWidget* parent) :
  QTreeView(parent),
  model(nullptr) {

  setIconSize(QSize(32, 32));
  setSelectionMode(QAbstractItemView::SingleSelection);
  setHeaderHidden(true);

  create_animation_action = new QAction(
        QIcon(":/images/icon_add.png"), tr("Create animation"), this);
  connect(create_animation_action, SIGNAL(triggered()),
          this, SIGNAL(create_animation_requested()));
  addAction(create_animation_action);

  create_direction_action = new QAction(
        QIcon(":/images/icon_add.png"), tr("Create direction"), this);
  connect(create_direction_action, SIGNAL(triggered()),
          this, SIGNAL(create_direction_requested()));
  addAction(create_direction_action);

  rename_animation_action = new QAction(
        QIcon(":/images/icon_rename.png"), tr("Rename animation"), this);
  rename_animation_action->setShortcut(tr("F2"));
  rename_animation_action->setShortcutContext(Qt::WidgetShortcut);
  connect(rename_animation_action, SIGNAL(triggered()),
          this, SIGNAL(rename_animation_requested()));
  addAction(rename_animation_action);

  duplicate_action = new QAction(
        QIcon(":/images/icon_copy.png"), tr("Duplicate..."), this);
  duplicate_action->setShortcutContext(Qt::WidgetShortcut);
  connect(duplicate_action, SIGNAL(triggered()),
          this, SIGNAL(duplicate_requested()));
  addAction(duplicate_action);

  delete_action = new QAction(
        QIcon(":/images/icon_delete.png"), tr("Delete..."), this);
  delete_action->setShortcut(QKeySequence::Delete);
  delete_action->setShortcutContext(Qt::WidgetShortcut);
  connect(delete_action, SIGNAL(triggered()),
          this, SIGNAL(delete_requested()));
  addAction(delete_action);
}

/**
 * @brief Shows a popup menu with actions related to the selected item.
 * @param event The event to handle.
 */
void SpriteTreeView::contextMenuEvent(QContextMenuEvent *event) {

  if (model == nullptr) {
    return;
  }

  QMenu* menu = new QMenu(this);
  menu->addAction(create_animation_action);

  SpriteModel::Index index = model->get_selected_index();
  if (index.is_valid()) {
    menu->addAction(create_direction_action);
    menu->addSeparator();
    menu->addAction(rename_animation_action);
    menu->addAction(duplicate_action);
    menu->addSeparator();
    menu->addAction(delete_action);
  }

  menu->popup(viewport()->mapToGlobal(event->pos()) + QPoint(1, 1));
}

/**
 * @brief Sets the sprite to represent in this view.
 * @param model The sprite model.
 */
void SpriteTreeView::set_model(SpriteModel* model) {

  this->model = model;
  SpriteTreeView::setModel(model);
  selectionModel()->deleteLater();
  setSelectionModel(&model->get_selection_model());
}

