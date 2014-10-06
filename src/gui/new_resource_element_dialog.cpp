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
#include "gui/new_resource_element_dialog.h"
#include "ui_new_resource_element_dialog.h"

/**
 * @brief Creates a dialog for a new resource element.
 * @param parent parent The parent widget or nullptr.
 */
NewResourceElementDialog::NewResourceElementDialog(QWidget* parent) :
  QDialog(parent),
  ui(new Ui::NewResourceElementDialog) {

  ui->setupUi(this);
}

/**
 * @brief Executes the dialog with the specified parameters.
 *
 * The dialog is modal. This function returns when the dialog is closed.
 *
 * @param resource_type The type of resource to create.
 * @return The information input by the user.
 */
NewResourceElementDialog::Result NewResourceElementDialog::exec(
    ResourceType resource_type) {

  QDialog::exec();
  // TODO
  return Result();
}
