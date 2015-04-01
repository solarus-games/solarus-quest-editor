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
#ifndef SOLARUSEDITOR_CHANGE_STRING_KEY_DIALOG_H
#define SOLARUSEDITOR_CHANGE_STRING_KEY_DIALOG_H

#include "ui_change_string_key_dialog.h"
#include <QDialog>

class StringsModel;

/**
 * @brief A dialog to change key of string in the strings editor.
 *
 * This dialog is similar to a standard QInputDialog but with two line edit.
 */
class ChangeStringKeyDialog : public QDialog {
  Q_OBJECT

public:

  ChangeStringKeyDialog(
      StringsModel* model, const QString& initial_key,
      bool is_prefix = false, bool allow_prefix = false, QWidget* parent = 0);

  QString get_string_key() const;
  void set_string_key(const QString& key);

  bool get_prefix() const;
  void set_prefix(bool prefix);

public slots:

  virtual void done(int result) override;

private:

  Ui::ChangeStringKeyDialog ui;   /**< The widgets. */
  QString initial_key;
  StringsModel* model;

};

#endif
