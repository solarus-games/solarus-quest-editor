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
#include "widgets/new_entity_user_property_dialog.h"
#include "widgets/gui_tools.h"
#include "entities/entity_model.h"

namespace SolarusEditor {

/**
 * @brief Creates a new entity user property dialog.
 * @param key Initial key of the new property.
 * @param value Initial value of the new property.
 * @param parent Parent object or nullptr.
 */
NewEntityUserPropertyDialog::NewEntityUserPropertyDialog (
  const QString& key, const QString &value, QWidget *parent) :
  QDialog(parent) {

  ui.setupUi(this);
  set_property(QPair<QString, QString>(key, value));
}

/**
 * @brief Returns the property key entered by the user.
 * @return The property.
 */
QPair<QString, QString> NewEntityUserPropertyDialog::get_property() const {

  return QPair<QString, QString>(
    ui.property_key_field->text(), ui.property_value_field->text());
}

/**
 * @brief Sets the property displayed in the text edit.
 * @param property The property to set.
 */
void NewEntityUserPropertyDialog::set_property(
    const QPair<QString, QString> &property) {

  ui.property_key_field->setText(property.first);
  ui.property_value_field->setText(property.second);
}

/**
 * @brief Closes the dialog unless the user tries to set invalid data.
 * @param result Result code of the dialog.
 */
void NewEntityUserPropertyDialog::done(int result) {

  if (result == QDialog::Accepted) {

    QString key = ui.property_key_field->text();

    if (!EntityModel::is_valid_user_property_key(key)) {
      GuiTools::error_dialog("Invalid user property key");
      return;
    }
  }

  QDialog::done(result);
}

}
