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
#ifndef SOLARUSEDITOR_QUEST_TREE_VIEW_H
#define SOLARUSEDITOR_QUEST_TREE_VIEW_H

#include "quest_manager.h"
#include <QPointer>
#include <QTreeView>

class QuestFilesModel;
class MainWindow;

/**
 * @brief A view of the quest files.
 */
class QuestTreeView : public QTreeView {
  Q_OBJECT

public:

  explicit QuestTreeView(QWidget* parent = nullptr);

  void set_quest_manager(QuestManager& quest_manager);

signals:

  void open_file_requested(Quest& quest, const QString& path);

protected:

  virtual void contextMenuEvent(QContextMenuEvent* event) override;
  virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

public slots:

  void current_quest_changed(Quest& quest);

  void open_action_triggered(const QString& path);
  void new_element_action_triggered(int resource_type);
  void new_directory_action_triggered(const QString& parent_path);
  void new_script_action_triggered(const QString& parent_path);
  void rename_action_triggered(const QString& path);
  void change_description_action_triggered(const QString& path);
  void delete_action_triggered(const QString& path);

private:

  void build_context_menu_new(QMenu& menu, const QString& path);
  void build_context_menu_open(QMenu& menu, const QString& path);
  void build_context_menu_rename(QMenu& menu, const QString& path);
  void build_context_menu_delete(QMenu& menu, const QString& path);

  QuestFilesModel* model;                  /**< The underlying model. */

};

#endif
