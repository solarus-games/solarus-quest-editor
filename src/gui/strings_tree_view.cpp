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
#include "gui/strings_tree_view.h"
#include "strings_model.h"
#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>

/**
 * @brief Creates an empty strings tree view.
 * @param parent The parent object or nullptr.
 */
StringsTreeView::StringsTreeView(QWidget* parent) :
  QTreeView(parent),
  model(nullptr) {

  setSelectionMode(QAbstractItemView::SingleSelection);
  setAlternatingRowColors(true);

  set_key_action = new QAction(
        QIcon(":/images/icon_rename.png"), tr("Change key..."), this);
  set_key_action->setShortcut(tr("F2"));
  set_key_action->setShortcutContext(Qt::WidgetShortcut);
  connect(set_key_action, SIGNAL(triggered()),
          this, SIGNAL(set_string_key_requested()));
  addAction(set_key_action);

  delete_action = new QAction(
        QIcon(":/images/icon_delete.png"), tr("Delete..."), this);
  delete_action->setShortcut(QKeySequence::Delete);
  delete_action->setShortcutContext(Qt::WidgetShortcut);
  connect(delete_action, SIGNAL(triggered()),
          this, SIGNAL(delete_string_requested()));
  addAction(delete_action);
}

/**
 * @brief Shows a popup menu with actions related to the selected item.
 * @param event The event to handle.
 */
void StringsTreeView::contextMenuEvent(QContextMenuEvent *event) {

  if (model == nullptr) {
    return;
  }

  QString key = model->get_selected_key();
  if (!model->prefix_exists(key)) {
    return;
  }

  QMenu* menu = new QMenu(this);
  menu->addAction(set_key_action);
  menu->addSeparator();
  menu->addAction(delete_action);
  menu->popup(viewport()->mapToGlobal(event->pos()) + QPoint(1, 1));
}

/**
 * @brief Sets the strings to represent in this view.
 * @param model The strings model.
 */
void StringsTreeView::set_model(StringsModel* model) {

  this->model = model;
  StringsTreeView::setModel(model);
  selectionModel()->deleteLater();
  setSelectionModel(&model->get_selection_model());
}

