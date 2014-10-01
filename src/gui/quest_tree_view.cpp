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
#include "gui/quest_tree_view.h"
#include "gui/quest_files_model.h"
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
  connect(this->quest_manager, SIGNAL(current_quest_changed(Quest&)),
          this, SLOT(current_quest_changed(Quest&)));
}

/**
 * @brief Slot called when the user loads another quest.
 * @param quest The current quest.
 */
void QuestTreeView::current_quest_changed(Quest& quest) {

  // Clean the old tree.
  setModel(nullptr);
  setSortingEnabled(false);

  // Create a new model.
  QuestFilesModel* model = new QuestFilesModel(quest);
  setModel(model);
  setRootIndex(model->get_quest_root_index());

  if (model->hasChildren(rootIndex())) {
    expand(rootIndex().child(0, 0));  // Expand the data directory.
  }

  sortByColumn(0, Qt::AscendingOrder);
  setColumnWidth(QuestFilesModel::FILE_COLUMN, 200);
  setColumnWidth(QuestFilesModel::DESCRIPTION_COLUMN, 200);

  // It is better for performance to enable sorting only after the model is ready.
  setSortingEnabled(true);
}
