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
#ifndef SOLARUSEDITOR_EDITOR_TABS_H
#define SOLARUSEDITOR_EDITOR_TABS_H

#include "quest_resources.h"
#include <QMap>
#include <QPointer>
#include <QTabWidget>

template<typename T> class QSet;
class QUndoGroup;

namespace SolarusEditor {

class Editor;
class Quest;
class Refactoring;

/**
 * \brief The main tab widget with all editors currently open.
 */
class EditorTabs : public QTabWidget {
  Q_OBJECT

public:

  EditorTabs(QWidget* parent = nullptr);

  QUndoGroup& get_undo_group();

  void open_resource(
      Quest& quest, ResourceType resource_type, const QString& id);
  void open_quest_properties_editor(Quest& quest);
  void open_map_editor(
      Quest& quest, const QString& path);
  void open_tileset_editor(
      Quest& quest, const QString& path);
  void open_sprite_editor(
      Quest& quest, const QString& path);
  void open_text_editor(
      Quest& quest, const QString& path);
  void open_dialogs_editor(
      Quest& quest, const QString& language_id);
  void open_strings_editor(
      Quest& quest, const QString& language_id);

  int find_editor(const QString& path);
  bool show_editor(const QString& path);

  Editor* get_editor(int index);
  Editor* get_editor();

  bool confirm_before_closing();
  bool has_unsaved_files();
  bool has_unsaved_files_other_than(const QSet<QString>& ignored_paths);
  QStringList get_unsaved_files();
  void close_without_confirmation();

  void reload_settings();

signals:

  void can_cut_changed(bool can_cut);
  void can_copy_changed(bool can_copy);
  void can_paste_changed(bool can_paste);
  void refactoring_requested(const Refactoring& refactoring);

public slots:

  bool save_file_requested(int index);
  bool save_all_files_requested();
  void close_file_requested(int index);
  void close_all_files_requested();
  void open_file_requested(Quest& quest, const QString& path);
  void reload_file_requested(int index);
  void file_renamed(const QString& old_path, const QString& new_path);
  void file_deleted(const QString& path);

protected:

  void keyPressEvent(QKeyEvent* event) override;
  void tabInserted(int index) override;
  void tabRemoved(int index) override;

private slots:

  void current_editor_changed(int index);
  void update_recent_files_list();
  void current_editor_modification_state_changed(bool clean);
  void modification_state_changed(int index, bool clean);

private:

  void add_editor(Editor* editor);
  void insert_editor(Editor* editor, int index);
  void remove_editor(int index);

  QMap<QString, Editor*> editors;      /**< All editors currently open,
                                        * indexed by their file path. */
  QUndoGroup* undo_group;              /**< Undo/redo stacks of open files. */
};

}

#endif
