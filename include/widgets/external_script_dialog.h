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
#ifndef SOLARUSEDITOR_EXTERNAL_SCRIPT_DIALOG_H
#define SOLARUSEDITOR_EXTERNAL_SCRIPT_DIALOG_H

#include "ui_external_script_dialog.h"
#include <QDialog>

/**
 * @brief A dialog that runs an external Lua script and shows its output.
 *
 * The script runs in a normal Lua environment with standard libraries.
 * io.write() outputs in the text area of this dialog instead of stdout.
 * For require(), everything works as if the current directory was the one
 * containing the script file.
 * This even works if the script file is located in Qt resources: in this
 * case, the script can require() other scripts that are also Qt resources,
 * using a relative path.
 */
class ExternalScriptDialog : public QDialog {
  Q_OBJECT

public:

  ExternalScriptDialog(const QString& title, const QString& script_path,
                       const QString& script_arg, QWidget* parent = 0);

  bool is_finished() const;
  bool is_successful() const;

public slots:

  virtual int exec() override;

protected:

  virtual void closeEvent(QCloseEvent* event) override;

private slots:

  void run_script();

private:

  void set_finished(bool finished);

  Ui::ExternalScriptDialog ui;         /**< The widgets. */
  QString script_path;                 /**< Lua script to run, without extension. */
  QString script_arg;                  /**< Optional argument to pass to the script. */
  bool finished;                       /**< Whether the script is finished. */
  bool successful;                     /**< Whether the script is successfully finished. */

};

#endif
