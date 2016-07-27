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
#ifndef GET_ANIMATION_NAME_DIALOG_H
#define GET_ANIMATION_NAME_DIALOG_H

#include <QInputDialog>

namespace SolarusEditor {

class SpriteModel;

/**
 * @brief A dialog to get an animation name in the sprite editor.
 */
class GetAnimationNameDialog : public QInputDialog {
  Q_OBJECT

public:

  GetAnimationNameDialog(const SpriteModel& model, QWidget* parent = 0);
  GetAnimationNameDialog(
      const SpriteModel& model,
      const QString& animation_name, QWidget* parent = 0);

  QString get_animation_name() const;
  void set_animation_name(const QString& animation_name);

public slots:

  virtual void done(int result) override;

private:

  void setup_ui ();

  const SpriteModel& model;
  QString animation_name;
};

}

#endif
