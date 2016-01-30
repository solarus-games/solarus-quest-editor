/*
 * Copyright (C) 2014-2016 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_NEW_RESOURCE_ELEMENT_DIALOG_H
#define SOLARUSEDITOR_NEW_RESOURCE_ELEMENT_DIALOG_H

#include "quest_resources.h"
#include "ui_new_resource_element_dialog.h"
#include <QDialog>
#include <QPair>

/**
 * @brief Dialog shown when creating a new resource element in the quest.
 *
 * This dialog does not create any resource element.
 * It only shows input widgets and provides the text entered by the user.
 */
class NewResourceElementDialog : public QDialog {
  Q_OBJECT

public:

  NewResourceElementDialog(ResourceType resource_type, QWidget* parent = nullptr);

  ResourceType get_resource_type() const;
  QString get_element_id() const;
  void set_element_id(const QString& value);
  QString get_element_description() const;
  void set_element_description(const QString& value);

public slots:

  void done(int result) override;

private:

  Ui::NewResourceElementDialog ui;     /**< The widgets. */

  ResourceType resource_type;          /**< Type of resource to create. */

};

#endif
