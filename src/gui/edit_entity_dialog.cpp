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
#include "include/gui/edit_entity_dialog.h"

/**
 * @brief Creates an edit entity dialog.
 * @param quest The quest.
 * @param parent The parent widget or nullptr.
 */
EditEntityDialog::EditEntityDialog(Quest& quest, QWidget* parent) :
  QDialog(parent) {

  ui.setupUi(this);

  ui.sprite_field->set_quest(quest);
  ui.sprite_field->set_resource_type(ResourceType::SPRITE);
  ui.treasure_name_field->set_quest(quest);
  ui.treasure_name_field->set_resource_type(ResourceType::ITEM);
}
