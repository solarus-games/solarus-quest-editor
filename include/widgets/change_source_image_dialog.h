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
#ifndef CHANGE_SOURCE_IMAGE_DIALOG_H
#define CHANGE_SOURCE_IMAGE_DIALOG_H

#include "ui_change_source_image_dialog.h"
#include <QDialog>
#include <QFileSystemModel>
#include <QGraphicsPixmapItem>

namespace SolarusEditor {

class Quest;

/**
 * @brief A dialog to change a source image in the sprite editor.
 */
class ChangeSourceImageDialog : public QDialog {
  Q_OBJECT

public:

  ChangeSourceImageDialog(
      const Quest& quest, const QString& initial_source_image,
      QWidget* parent = 0);

  QString get_source_image() const;
  void set_source_image(const QString& source_image);

  void update_source_image();
  void update_image_view();

public slots:

  virtual void done(int result) override;

  void change_is_tileset_requested();
  void change_source_image_requested();

private:

  Ui::ChangeSourceImageDialog ui;     /**< The widgets. */
  QString image_path;                 /**< The sprites image path of the quest. */
  QFileSystemModel file_system_model; /**< The file system model. */
  QString source_image;               /**< The source image. */
  QGraphicsScene scene;               /**< The graphics scene of image. */
  QGraphicsPixmapItem image_item;     /**< The image item of the graphics scene. */

};

}

#endif
