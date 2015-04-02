/*
 * Copyright (C) 2014-2015 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_STRINGS_TREE_VIEW_H
#define SOLARUSEDITOR_STRINGS_TREE_VIEW_H

#include <QTreeView>

class StringsModel;

/**
 * @brief Tree view of a strings.
 *
 * This tree view shows all strings of a language.
 */
class StringsTreeView : public QTreeView {
  Q_OBJECT

public:

  StringsTreeView(QWidget* parent = nullptr);

  void set_model(StringsModel *model);

signals:

  void create_string_requested();
  void set_string_key_requested();
  void delete_string_requested();

protected:

  virtual void contextMenuEvent(QContextMenuEvent* event) override;

private:

  StringsModel*
    model;          /**< The strings model. */
  QAction*
    create_action;  /**< Action of create a new string(s). */
  QAction*
    set_key_action; /**< Action of change the key of the selected string(s). */
  QAction*
    delete_action;  /**< Action of deleting the selected string(s). */

};

#endif
