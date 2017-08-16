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
#ifndef SOLARUSEDITOR_DIALOG_PROPERTIES_TABLE_H
#define SOLARUSEDITOR_DIALOG_PROPERTIES_TABLE_H

#include <QTreeWidget>

namespace SolarusEditor {

class DialogsModel;

/**
 * @brief Table view of a dialog properties.
 */
class DialogPropertiesTable : public QTreeWidget {
  Q_OBJECT

public:

  DialogPropertiesTable(QWidget* parent = nullptr);

  void set_model(DialogsModel *model);

  QString get_selected_property() const;
  void set_selected_property(const QString& key);

public slots:

  void update();
  void dialog_property_created(
      const QString& id, const QString& key, const QString& value);
  void dialog_property_deleted(const QString& id, const QString& key);
  void dialog_property_value_changed(
      const QString& id, const QString& key, const QString& new_value);

  void on_item_double_clicked(QTreeWidgetItem *item, int column);
  void on_item_changed(QTreeWidgetItem *item, int column);

signals:

  void create_property_requested();
  void set_property_key_requested();
  void set_property_value_requested(const QString& key, const QString& value);
  void delete_property_requested();
  void set_from_translation_requested();

protected:

  virtual void contextMenuEvent(QContextMenuEvent* event) override;

private:

  static const int KEY_COLUMN = 0;
  static const int VALUE_COLUMN = 1;
  static const int TRANSLATION_COLUMN = 2;

  void clear_table();
  void add_translation_property(const QString& key, const QString& value);

  DialogsModel*
    model;          /**< The dialogs model. */
  QString dialog_id;

  QMap<QString, QTreeWidgetItem*> items;

  QAction*
    create_action;  /**< Action of create a new property. */
  QAction*
    set_key_action; /**< Action of change the key of the selected property. */
  QAction*
    delete_action;  /**< Action of deleting the selected property. */
  QAction*
    set_action;     /**< Action of setting the selected property from translation. */

};

}

#endif
