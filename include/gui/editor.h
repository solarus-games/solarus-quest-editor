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

/**
 * \brief Abstract class for a widget that can edit something in Solarus.
 *
 * It has to support common operations like saving and closing.
 * This widget is initialized with a layout and margins, so that
 * subclasses can add a child widget of their choice.
 */
class Editor : public QWidget {
  Q_OBJECT

public:

  Editor(Quest& quest, const QString& file_path, QWidget* parent = nullptr);

  Quest& get_quest();
  QString get_file_path() const;
  QString get_file_name() const;

  virtual QString get_title() const = 0;
  virtual QIcon get_icon() const = 0;
  virtual bool is_modified() const = 0;
  virtual void save() = 0;
  virtual bool confirm_close() = 0;

signals:

  void modification_state_changed(bool modified);

private:

  Quest& quest;        /**< The quest the edited file belongs to. */
  QString file_path;   /**< Path of the edited file. */

};

#endif
