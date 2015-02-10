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
#ifndef NEW_ANIMATION_DIALOG_H
#define NEW_ANIMATION_DIALOG_H

#include "ui_new_animation_dialog.h"
#include <QDialog>

/**
 * @brief A dialog to create a new animation in the sprite editor.
 *
 * This dialog is similar to a standard QInputDialog with a line edit.
 */
class NewAnimationDialog : public QDialog {
  Q_OBJECT

public:

  NewAnimationDialog(QWidget* parent = 0);

  QString get_animation_name() const;
  void set_animation_name(const QString& animation_name);

public slots:

  virtual void done(int result) override;

private:

  Ui::NewAnimationDialog ui;   /**< The widgets. */

};

#endif
