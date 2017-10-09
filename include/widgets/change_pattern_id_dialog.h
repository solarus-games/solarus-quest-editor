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
#ifndef CHANGE_PATTERN_ID_DIALOG_H
#define CHANGE_PATTERN_ID_DIALOG_H

#include "ui_change_pattern_id_dialog.h"
#include <QDialog>

namespace SolarusEditor {

/**
 * @brief A dialog to rename a pattern in the tileset editor.
 *
 * This dialog is similar to a standard QInputDialog with a line edit,
 * but has an additional checkbox to let the user choose if she wants
 * to update references in existing maps.
 */
class ChangePatternIdDialog : public QDialog {
  Q_OBJECT

public:

  ChangePatternIdDialog(
      const QString& initial_pattern_id, QWidget* parent = 0);

  QString get_pattern_id() const;
  void set_pattern_id(const QString& pattern_id);

  bool get_update_references() const;
  void set_update_references(bool update_references);

public slots:

  void done(int result) override;

private:

  Ui::ChangePatternIdDialog ui;   /**< The widgets. */

};

}

#endif
