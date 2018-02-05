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
#ifndef SOLARUSEDITOR_NEW_ENTITY_USER_PROPERTY_DIALOG_H
#define SOLARUSEDITOR_NEW_ENTITY_USER_PROPERTY_DIALOG_H

#include "ui_new_entity_user_property_dialog.h"
#include <QDialog>

namespace SolarusEditor {

/**
 * @brief A dialog to create a new user property of an entity.
 *
 * This dialog is similar to a standard QInputDialog but with two line edit.
 */
class NewEntityUserPropertyDialog : public QDialog {
  Q_OBJECT

public:

  explicit NewEntityUserPropertyDialog(
      const QString& key, const QString& value = "", QWidget* parent = nullptr);

  QPair<QString, QString> get_property() const;
  void set_property(const QPair<QString, QString> &property);

public slots:

  virtual void done(int result) override;

private:

  Ui::NewEntityUserPropertyDialog ui;   /**< The widgets. */

};

}

#endif
