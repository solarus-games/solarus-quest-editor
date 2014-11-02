/*
 * Copyright (C) 2014 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef EXTERNAL_SCRIPT_DIALOG_H
#define EXTERNAL_SCRIPT_DIALOG_H

#include "ui_external_script_dialog.h"
#include <QDialog>

/**
 * @brief A dialog that runs an external Lua script and shows its output.
 */
class ExternalScriptDialog : public QDialog {
  Q_OBJECT

public:

  ExternalScriptDialog(const QString& title, const QString& script_path,
                       const QString& arg, QWidget* parent = 0);

  bool is_finished() const;
  bool is_successful() const;

protected:

  virtual void closeEvent(QCloseEvent* event) override;

private:

  Ui::ExternalScriptDialog ui;

};

#endif
