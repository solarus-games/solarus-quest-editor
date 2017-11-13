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
#ifndef SOLARUSEDITOR_CHANGE_BORDER_SET_ID_DIALOG_H
#define SOLARUSEDITOR_CHANGE_BORDER_SET_ID_DIALOG_H

#include "ui_change_border_set_id_dialog.h"
#include <QDialog>

namespace SolarusEditor {

/**
 * @brief A dialog to rename a border set id in the tileset editor.
 */
class ChangeBorderSetIdDialog : public QDialog {
  Q_OBJECT

public:

  explicit ChangeBorderSetIdDialog(
      const QString& initial_border_set_id,
      QWidget* parent = nullptr
  );

  QString get_border_set_id() const;
  void set_border_set_id(const QString& border_set_id);

public slots:

  void done(int result) override;

private:

  Ui::ChangeBorderSetIdDialog ui;   /**< The widgets. */

};

}

#endif
