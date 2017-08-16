/*
 * Copyright (C) 2014-2017 Christopho, Solarus - http://www.solarus-games.org
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
#include "widgets/dialogs_tree_view.h"
#include "dialogs_model.h"
#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>

namespace SolarusEditor {

/**
 * @brief Creates an empty dialogs tree view.
 * @param parent The parent object or nullptr.
 */
DialogsTreeView::DialogsTreeView(QWidget* parent) :
  QTreeView(parent),
  model(nullptr) {

  setSelectionMode(QAbstractItemView::SingleSelection);
  setHeaderHidden(true);

  create_action = new QAction(
        QIcon(":/images/icon_add.png"), tr("New dialog..."), this);
  connect(create_action, SIGNAL(triggered()),
          this, SIGNAL(create_dialog_requested()));
  addAction(create_action);

  duplicate_action = new QAction(
        QIcon(":/images/icon_copy.png"), tr("Duplicate dialog(s)..."), this);
  connect(duplicate_action, SIGNAL(triggered()),
          this, SIGNAL(duplicate_dialog_requested()));
  addAction(duplicate_action);

  set_id_action = new QAction(
        QIcon(":/images/icon_rename.png"), tr("Change id..."), this);
  set_id_action->setShortcut(tr("F2"));
  set_id_action->setShortcutContext(Qt::WidgetShortcut);
  connect(set_id_action, SIGNAL(triggered()),
          this, SIGNAL(set_dialog_id_requested()));
  addAction(set_id_action);

  delete_action = new QAction(
        QIcon(":/images/icon_delete.png"), tr("Delete..."), this);
  delete_action->setShortcut(QKeySequence::Delete);
  delete_action->setShortcutContext(Qt::WidgetShortcut);
  connect(delete_action, SIGNAL(triggered()),
          this, SIGNAL(delete_dialog_requested()));
  addAction(delete_action);
}

/**
 * @brief Shows a popup menu with actions related to the selected item.
 * @param event The event to handle.
 */
void DialogsTreeView::contextMenuEvent(QContextMenuEvent *event) {

  if (model == nullptr) {
    return;
  }

  QMenu* menu = new QMenu(this);
  menu->addAction(create_action);

  QString id = model->get_selected_id();
  if (model->prefix_exists(id)) {
    menu->addSeparator();
    menu->addAction(set_id_action);
    menu->addAction(duplicate_action);
    menu->addSeparator();
    menu->addAction(delete_action);
  }

  menu->popup(viewport()->mapToGlobal(event->pos()) + QPoint(1, 1));
}

/**
 * @brief Sets the dialogs to represent in this view.
 * @param model The dialogs model.
 */
void DialogsTreeView::set_model(DialogsModel* model) {

  this->model = model;
  DialogsTreeView::setModel(model);
  selectionModel()->deleteLater();
  setSelectionModel(&model->get_selection_model());
}

}
