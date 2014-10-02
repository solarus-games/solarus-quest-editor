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
#ifndef SOLARUSEDITOR_EDITOR_TABS_H
#define SOLARUSEDITOR_EDITOR_TABS_H

#include <solarus/ResourceType.h>
#include <QMap>
#include <QTabWidget>

class Editor;
class Quest;
class QuestManager;

/**
 * \brief The main tab widget with all editors currently open.
 */
class EditorTabs : public QTabWidget {
  Q_OBJECT

public:

  using ResourceType = Solarus::ResourceType;

  EditorTabs(QWidget* parent = nullptr);

  void set_quest_manager(QuestManager& quest_manager);

  void open_resource(
      Quest& quest, ResourceType resource_type, const QString& id);
  void open_script(
      Quest& quest, const QString& path);

  int find_editor(const QString& path);
  bool show_editor(const QString& path);

public slots:

  void close_file_requested(int index);
  void open_file_requested(Quest& quest, const QString& path);

private:

  void add_editor(Editor* editor,
                  const QString& icon_name);
  void remove_editor(int index);

  QMap<QString, QWidget*> editors;     /**< All editors currently open,
                                        * indexed by their file path. */
};

#endif
