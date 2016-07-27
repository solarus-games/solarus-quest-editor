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
#include "widgets/find_text_dialog.h"
#include <QPushButton>

namespace SolarusEditor {

/**
 * @brief Creates a find text dialog.
 * @param parent The parent object or nullptr.
 */
FindTextDialog::FindTextDialog(QWidget* parent) :
  QDialog(parent),
  ui() {

  ui.setupUi(this);

  QPushButton* find_button = new QPushButton(tr("Find"), this);
  ui.button_box->addButton(find_button, QDialogButtonBox::ApplyRole);

  find_button->setDefault(true);

  connect(find_button, &QPushButton::pressed, [this]() {
    emit find_text_requested(ui.find_field->text());
  });
}

}
