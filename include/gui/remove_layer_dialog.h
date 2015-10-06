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
#ifndef SOLARUSEDITOR_REMOVE_LAYER_DIALOG_H
#define SOLARUSEDITOR_REMOVE_LAYER_DIALOG_H

#include "ui_remove_layer_dialog.h"

/**
 * @brief Dialogs asking what to do with entities layers about to be removed.
 */
class RemoveLayerDialog : public QDialog {
  Q_OBJECT

public:

  explicit RemoveLayerDialog(QWidget* parent = nullptr);

private:
  Ui::RemoveLayerDialog ui;

};

#endif
