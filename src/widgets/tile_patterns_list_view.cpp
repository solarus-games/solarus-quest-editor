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
#include "widgets/tile_patterns_list_view.h"
#include "tileset_model.h"
#include <QAction>

/**
 * @brief Creates an empty tile patterns list view.
 * @param parent The parent object or nullptr.
 */
TilePatternsListView::TilePatternsListView(QWidget* parent) :
  QListView(parent) {

  setIconSize(QSize(32, 32));
  setUniformItemSizes(true);
  setViewMode(IconMode);
  setResizeMode(Adjust);
  setSpacing(10);
  setSelectionMode(QAbstractItemView::ExtendedSelection);

  QAction* action;

  action = new QAction(tr("Delete..."), this);
  action->setShortcut(QKeySequence::Delete);
  action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(action, SIGNAL(triggered()),
          this, SIGNAL(delete_selected_patterns_requested()));
  addAction(action);

  action = new QAction(tr("Change id..."), this);
  action->setShortcut(tr("F2"));
  action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(action, SIGNAL(triggered()),
          this, SIGNAL(change_selected_pattern_id_requested()));
  addAction(action);

}

/**
 * @brief Sets the tileset to represent in this view.
 * @param model The tileset model.
 */
void TilePatternsListView::set_model(TilesetModel& model) {

  QListView::setModel(&model);
  selectionModel()->deleteLater();
  setSelectionModel(&model.get_selection_model());
}
