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
#ifndef SOLARUSEDITOR_NEW_RESOURCE_ELEMENT_DIALOG_H
#define SOLARUSEDITOR_NEW_RESOURCE_ELEMENT_DIALOG_H

#include "ui_new_resource_element_dialog.h"
#include <solarus/ResourceType.h>
#include <QDialog>
#include <QPair>

/**
 * @brief Dialog shown when creating a new resource element in the quest.
 */
class NewResourceElementDialog : public QDialog {
  Q_OBJECT

public:

  using ResourceType = Solarus::ResourceType;

  /**
   * @brief Information entered by the user.
   */
  struct Result {
    bool validated;          /**< Whether the user has validated or canceled. */
    QString id;              /**< Id entered by the user. */
    QString description;     /**< Description entered by the user. */
  };

  NewResourceElementDialog(QWidget* parent = nullptr);

  Result exec(ResourceType resource_type);

private:

  Ui::NewResourceElementDialog ui;

};

#endif
