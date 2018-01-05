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
#ifndef SOLARUSEDITOR_DIALOGS_TREE_VIEW_H
#define SOLARUSEDITOR_DIALOGS_TREE_VIEW_H

#include <QTreeView>

namespace SolarusEditor {

class DialogsModel;

/**
 * @brief Tree view of a dialogs.
 *
 * This tree view shows all dialogs of a language.
 */
class DialogsTreeView : public QTreeView {
  Q_OBJECT

public:

  explicit DialogsTreeView(QWidget* parent = nullptr);

  void set_model(DialogsModel *model);

signals:

  void create_dialog_requested();
  void duplicate_dialog_requested();
  void set_dialog_id_requested();
  void delete_dialog_requested();

protected:

  virtual void contextMenuEvent(QContextMenuEvent* event) override;

private:

  DialogsModel*
    model;            /**< The dialogs model. */
  QAction*
    create_action;    /**< Action of create a new dialog. */
  QAction*
    duplicate_action; /**< Action of duplicate dialog(s). */
  QAction*
    set_id_action;    /**< Action of change the id of the selected dialog(s). */
  QAction*
    delete_action;    /**< Action of deleting the selected dialog(s). */

};

}

#endif
