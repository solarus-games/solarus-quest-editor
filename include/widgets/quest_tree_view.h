/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
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

#include <QPointer>
#include <QTreeView>

namespace SolarusEditor {

class Quest;
class QuestFilesModel;
class MainWindow;

/**
 * @brief A hierachical view of files of a quest.
 */
class QuestTreeView : public QTreeView {
  Q_OBJECT

public:

  explicit QuestTreeView(QWidget* parent = nullptr);

  void set_quest(Quest& quest);

  bool is_read_only() const;
  void set_read_only(bool read_only);
  bool is_opening_files_allowed() const;
  void set_opening_files_allowed(bool opening_files_allowed);

  QString get_selected_path() const;
  void set_selected_path(const QString& path);
  void add_selected_path(const QString& path);
  QStringList get_selected_paths() const;
  void set_selected_paths(const QStringList& paths);

signals:

  void open_file_requested(Quest& quest, const QString& path);
  void rename_file_requested(Quest& quest, const QString& path);
  void selected_path_changed(const QString& path);

public slots:

  void default_action_triggered();
  void new_element_action_triggered();
  void new_directory_action_triggered();
  void new_script_action_triggered();
  void play_action_triggered();
  void open_action_triggered();
  void open_map_script_action_triggered();
  void open_language_strings_action_triggered();
  void rename_action_triggered();
  void file_renamed(const QString& old_path, const QString& new_path);
  void change_description_action_triggered();
  void delete_action_triggered();

protected:

  void contextMenuEvent(QContextMenuEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

private:

  void build_context_menu_new(QMenu& menu, const QString& path);
  void build_context_menu_play(QMenu& menu, const QString& path);
  void build_context_menu_open(QMenu& menu, const QString& path);
  void build_context_menu_rename(QMenu& menu, const QString& path);
  void build_context_menu_delete(QMenu& menu, const QString& path);

  QuestFilesModel* model;         /**< The underlying model. */
  QAction* play_action;           /**< Action of playing or stopping the selected file. */
  QAction* open_action;           /**< Action of opening the selected file. */
  QAction* rename_action;         /**< Action of renaming the selected file. */
  QAction* delete_action;         /**< Action of deleting the selected file. */

  bool read_only;                 /**< Whether the view forbids editing the tree. */
  bool opening_files_allowed;     /**< Whether the user can open files from this tree. */

};

}

#endif
