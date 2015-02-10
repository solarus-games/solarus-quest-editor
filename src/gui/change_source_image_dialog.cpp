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
#include "gui/change_source_image_dialog.h"
#include "gui/gui_tools.h"
#include "quest.h"

/**
 * @brief Creates a change source image dialog.
 * @param initial_source_image Initial value of the source image.
 * @param parent Parent object or nullptr.
 */
ChangeSourceImageDialog::ChangeSourceImageDialog(
    const Quest &quest, const QString& initial_source_image,
    QWidget* parent) :
  QDialog(parent),
  image_path(quest.get_sprite_image_path("")) {

  ui.setupUi(this);

  file_system_model.setRootPath(image_path);
  file_system_model.setNameFilters(QStringList("*.png"));
  file_system_model.setNameFilterDisables(false);

  ui.file_tree_view->setModel(&file_system_model);
  ui.file_tree_view->setRootIndex(file_system_model.index(image_path));

  ui.file_tree_view->setSelectionMode(QAbstractItemView::SingleSelection);
  ui.file_tree_view->setHeaderHidden(true);
  ui.file_tree_view->setColumnHidden(1, true);
  ui.file_tree_view->setColumnHidden(2, true);
  ui.file_tree_view->setColumnHidden(3, true);

  scene.addItem(&image_item);
  ui.image_view->setScene(&scene);

  set_source_image(initial_source_image);

  connect(ui.tileset_value, SIGNAL(clicked()),
          this, SLOT(change_is_tileset_requested()));
  connect(ui.image_value, SIGNAL(clicked()),
          this, SLOT(change_is_tileset_requested()));

  connect(ui.file_tree_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(change_source_image_requested()));
}

/**
 * @brief Returns source image selected by the user.
 * @return The pattern id.
 */
QString ChangeSourceImageDialog::get_source_image() const {

  if (ui.tileset_value->isChecked()) {
    return "tileset";
  }
  return source_image;
}

/**
 * @brief Sets the source image.
 * @param value The value to set.
 */
void ChangeSourceImageDialog::set_source_image(const QString &source_image) {

  this->source_image = source_image;

  bool is_tileset = source_image == "tileset";
  ui.tileset_value->setChecked(is_tileset);
  ui.image_value->setChecked(!is_tileset);
  ui.file_tree_view->setEnabled(!is_tileset);

  update_source_image();
}

/**
 * @brief Update the source image selected in the tree.
 */
void ChangeSourceImageDialog::update_source_image() {

  QString path = image_path + source_image;
  QModelIndex index = file_system_model.index(path);
  ui.file_tree_view->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
  ui.file_tree_view->expand(index.parent());

  update_image_view();
}

/**
 * @brief Update the image view.
 */
void ChangeSourceImageDialog::update_image_view() {

  image_item.setPixmap(QPixmap::fromImage(QImage(image_path + source_image)));
  ui.image_view->update();
}

/**
 * @brief Closes the dialog unless the user tries to set invalid data.
 * @param result Result code of the dialog.
 */
void ChangeSourceImageDialog::done(int result) {

  if (result == QDialog::Accepted) {

    QString source_image = get_source_image();
    if (source_image.isEmpty()) {
      GuiTools::error_dialog(tr("No image selected."));
      return;
    }
  }

  QDialog::done(result);
}


/**
 * @brief Slot called when the user checks tileset or image combo box.
 */
void ChangeSourceImageDialog::change_is_tileset_requested() {

  bool is_tileset = ui.tileset_value->isChecked();

  source_image = is_tileset ? "tileset" : "";

  ui.file_tree_view->setEnabled(!is_tileset);
  update_source_image();
}

/**
 * @brief Slot called when the user changes the selected source image.
 */
void ChangeSourceImageDialog::change_source_image_requested() {

  QModelIndexList indexes =
      ui.file_tree_view->selectionModel()->selectedRows();

  if (indexes.size() != 1) {
    // Multiple selection.
    return;
  }

  source_image = file_system_model.filePath(indexes[0]);

  QFileInfo info(source_image);
  if (info.isFile()) {
    source_image.remove(image_path);
  } else {
    source_image = "";
  }

  update_image_view();
}
