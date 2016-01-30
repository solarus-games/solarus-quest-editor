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
#include "widgets/gui_tools.h"
#include "widgets/new_resource_element_dialog.h"
#include "quest.h"

/**
 * @brief Creates a dialog for a new resource element.
 * @param resource_type The type of resource to create.
 * @param parent parent The parent widget or nullptr.
 */
NewResourceElementDialog::NewResourceElementDialog(
    ResourceType resource_type, QWidget* parent) :
  QDialog(parent),
  resource_type(resource_type) {

  ui.setupUi(this);

  // Determine the text to show.
  // A switch is the best way to ensure at compile time that we don't
  // forget resource types, and to also handle translations correctly.
  QString title;
  QString id_text;
  switch (resource_type) {

  case ResourceType::MAP:
    title = tr("New map");
    id_text = tr("Map id (filename):");
    break;

  case ResourceType::TILESET:
    title = tr("New tileset");
    id_text = tr("Tileset id (filename):");
    break;

  case ResourceType::SPRITE:
    title = tr("New sprite");
    id_text = tr("Sprite id (filename):");
    break;

  case ResourceType::MUSIC:
    title = tr("New music");
    id_text = tr("Music id (filename):");
    break;

  case ResourceType::SOUND:
    title = tr("New sound");
    id_text = tr("Sound id (filename):");
    break;

  case ResourceType::ITEM:
    title = tr("New item");
    id_text = tr("Item id (filename):");
    break;

  case ResourceType::ENEMY:
    title = tr("New enemy");
    id_text = tr("Enemy id (filename):");
    break;

  case ResourceType::ENTITY:
    title = tr("New custom entity");
    id_text = tr("Custom entity id (filename):");
    break;

  case ResourceType::LANGUAGE:
    title = tr("New language");
    id_text = tr("Language id (filename):");
    break;

  case ResourceType::FONT:
    title = tr("New font");
    id_text = tr("Font id (filename):");
    break;

  }

  ui.id_label->setText(id_text);
  setWindowTitle(title);
}

/**
 * @brief Returns the type of resource to be created.
 * @return The type of resource.
 */
ResourceType NewResourceElementDialog::get_resource_type() const {
  return resource_type;
}

/**
 * @brief Returns resource element id entered by the user.
 * @return The resource element id.
 */
QString NewResourceElementDialog::get_element_id() const {

  return ui.id_line_edit->text();
}

/**
 * @brief Sets the value displayed in the resource element id text edit.
 * @param value The value to set.
 */
void NewResourceElementDialog::set_element_id(const QString& value) {

  ui.id_line_edit->setText(value);
}

/**
 * @brief Returns resource element description entered by the user.
 * @return The description.
 */
QString NewResourceElementDialog::get_element_description() const {

  return ui.description_line_edit->text();
}

/**
 * @brief Sets the value id displayed in the description text edit.
 * @param value The value to set.
 */
void NewResourceElementDialog::set_element_description(const QString& value) {

  ui.description_line_edit->setText(value);
}

/**
 * @brief Closes the dialog unless the user tries to set invalid data.
 * @param result Result code of the dialog.
 */
void NewResourceElementDialog::done(int result) {

  if (result == QDialog::Accepted) {

    if (get_element_id().isEmpty()) {
      GuiTools::error_dialog("Empty resource id");
      return;
    }

    if (!Quest::is_valid_file_name(get_element_id())) {
      GuiTools::error_dialog("Invalid resource id");
      return;
    }

    if (get_element_description().isEmpty()) {
      GuiTools::error_dialog("Empty resource description");
      return;
    }
  }

  QDialog::done(result);
}
