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
#include "quest_tree_view.h"
#include <QFileSystemModel>

/**
 * @brief Creates a quest tree view.
 * @param parent The parent or nullptr.
 */
QuestTreeView::QuestTreeView(QWidget* parent) :
  QTreeView(parent),
  quest_manager(nullptr) {

  setUniformRowHeights(true);
}

/**
 * @brief Sets the quest manager observed by this view.
 * @param quest_manager The quest manager.
 */
void QuestTreeView::set_quest_manager(QuestManager& quest_manager) {

  if (this->quest_manager != nullptr) {
    // Disconnect from the old quest manager.
    this->quest_manager->disconnect(this);
  }

  this->quest_manager = &quest_manager;

  // Connect to the new quest manager.
  connect(this->quest_manager, SIGNAL(current_quest_changed(QString)),
          this, SLOT(current_quest_changed(QString)));
}

/**
 * @brief Slot called when the user loads another quest.
 * @param quest_path Path of the current quest.
 */
void QuestTreeView::current_quest_changed(QString /* quest_path */) {

  // Clean the old tree.
  setModel(nullptr);
  setSortingEnabled(false);

  // Create a new model.
  QString quest_data_path = quest_manager->get_quest_data_path();
  QFileSystemModel* model = new QFileSystemModel;
  model->setRootPath(quest_data_path);
  model->setReadOnly(false);
  setModel(model);
  setRootIndex(model->index(quest_data_path));

  // It is better for performance to do this after setting the model.
  setSortingEnabled(true);
  sortByColumn(0, Qt::AscendingOrder);
}
