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

#include <QIcon>
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
  QString get_title() const;
  QIcon get_icon() const;
  QUndoStack& get_undo_stack();
  bool confirm_close();

  virtual void save() = 0;
  virtual void cut() = 0;
  virtual void copy() = 0;
  virtual void paste() = 0;

protected:

  void set_title(const QString& title);
  void set_icon(const QIcon& icon);

  QString get_close_confirm_message() const;
  void set_close_confirm_message(const QString& message);

private:

  Quest& quest;              /**< The quest the edited file belongs to. */
  QString file_path;         /**< Path of the edited file. */
  QString title;             /**< Title of the file. */
  QIcon icon;                /**< Icon representing the file. */
  QString
      close_confirm_message; /**< Message proposing to save changes when closing. */
  QUndoStack* undo_stack;    /**< The undo/redo history of editing this file. */

};

#endif
