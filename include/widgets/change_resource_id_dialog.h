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
#ifndef SOLARUSEDITOR_CHANGE_RESOURCE_ID_DIALOG_H
#define SOLARUSEDITOR_CHANGE_RESOURCE_ID_DIALOG_H

#include "quest_resources.h"
#include "ui_change_resource_id_dialog.h"
#include <QDialog>

namespace SolarusEditor {

class Quest;

class ChangeResourceIdDialog : public QDialog {
  Q_OBJECT

public:

  explicit ChangeResourceIdDialog(
      Quest& quest,
      ResourceType resource_type,
      const QString& old_id,
      QWidget* parent = nullptr);

  QString get_element_id();
  void set_element_id(const QString& element_id);

  bool get_update_references() const;
  void set_update_references(bool update_references);

public slots:

  void done(int result) override;

private:

  Ui::ChangeResourceIdDialog ui;   /**< The widgets. */

};


} // namespace SolarusEditor

#endif
