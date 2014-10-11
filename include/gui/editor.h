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
#ifndef SOLARUSEDITOR_EDITOR_H
#define SOLARUSEDITOR_EDITOR_H

#include <QWidget>

class Quest;
class QuestResources;
class QUndoStack;

/**
 * \brief Abstract class for a widget that can edit something in Solarus.
 *
 * It has to support common operations like saving and closing.
 */
class Editor : public QWidget {
  Q_OBJECT

public:

  Editor(Quest& quest, const QString& path, QWidget* parent = nullptr);

  const Quest& get_quest() const;
  Quest& get_quest();
  const QuestResources& get_resources() const;
  QuestResources& get_resources();
  QString get_file_path() const;
  QString get_file_name() const;
  QString get_file_name_without_extension() const;
  QUndoStack& get_undo_stack();

  virtual QString get_title() const;
  virtual QIcon get_icon() const = 0;
  virtual void save() = 0;
  virtual bool confirm_close() = 0;
  virtual void cut() = 0;
  virtual void copy() = 0;
  virtual void paste() = 0;

private:

  Quest& quest;              /**< The quest the edited file belongs to. */
  QString file_path;         /**< Path of the edited file. */
  QUndoStack* undo_stack;    /**< The undo/redo history of editing this file. */

};

#endif
