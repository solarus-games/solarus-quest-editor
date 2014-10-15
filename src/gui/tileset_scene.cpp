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
#include "gui/tileset_model.h"
#include "gui/tileset_scene.h"
#include "quest.h"
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QPalette>

/**
 * @brief TilesetScene Creates a tileset scene.
 * @param model The tileset data to represent in the scene.
 * @param parent The parent object or nullptr.
 */
TilesetScene::TilesetScene(TilesetModel& model, QObject* parent) :
  QGraphicsScene(parent),
  model(model) {

  build();

  // Synchronize the scene selection with the tileset selection model.
  connect(&model.get_selection(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(update_selection_to_scene(QItemSelection, QItemSelection)));
  connect(this, SIGNAL(selectionChanged()),
          this, SLOT(set_selection_from_scene()));
}

/**
 * @brief Returns the tileset represented in the scene.
 * @return The tileset.
 */
const TilesetModel& TilesetScene::get_model() const {
  return model;
}

/**
 * @brief Returns the quest the tileset belongs to.
 * @return The quest.
 */
const Quest& TilesetScene::get_quest() const {
  return model.get_quest();
}

/**
 * @brief Draws the tileset image as background.
 * @param painter The painter.
 * @param rect The exposed rectangle in scene coordinates.
 * It may be larger than the scene.
 */
void TilesetScene::drawBackground(QPainter* painter, const QRectF& rect) {

  // Draw the background color.
  painter->fillRect(rect, palette().window());

  // Draw the full PNG image of the tileset.
  const QImage& patterns_image = model.get_patterns_image();
  if (!patterns_image.isNull()) {
    painter->drawImage(0, 0, patterns_image);
  }
}

/**
 * @brief Create all patterns items in the scene.
 */
void TilesetScene::build() {

  clear();
  pattern_items.clear();

  if (model.get_patterns_image().isNull()) {
    // The tileset image does not exist yet.
    // Maybe this is a recently created tileset.
    QString path = get_quest().get_tileset_tiles_image_path(model.get_tileset_id());
    path = path.right(path.length() - get_quest().get_data_path().length() - 1);
    addText(tr("Missing tileset image '%1'").arg(path));
    return;
  }

  setSceneRect(QRectF(QPoint(0, 0), model.get_patterns_image().size()));
  for (int i = 0; i < model.get_num_patterns(); ++i) {
    QPixmap image = model.get_pattern_image(i);
    QRect frame = model.get_pattern_frame(i);

    // TODO make a function to create an item.
    QGraphicsPixmapItem* pattern_item = addPixmap(image);
    pattern_item->setPos(frame.topLeft());
    pattern_item->setFlags(
          QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    pattern_item->setData(0, i);
    pattern_items.append(pattern_item);
  }

}

/**
 * @brief Slot called when the tileset selection has changed.
 *
 * The selection on the scene is updated.
 *
 * @param selected Items that have just been selected.
 * @param deselected Item that have just been deselected.
 */
void TilesetScene::update_selection_to_scene(
    const QItemSelection& selected, const QItemSelection& deselected) {

  // Update the scene with the change.
  for (const QModelIndex& model_index: selected.indexes()) {
    int index = model_index.row();
    if (model.pattern_exists(index)) {
      pattern_items[index]->setSelected(true);
    }
  }

  for (const QModelIndex& model_index: deselected.indexes()) {
    int index = model_index.row();
    if (model.pattern_exists(index)) {
      pattern_items[index]->setSelected(false);
    }
  }
}

/**
 * @brief Slot called when the scene selection has changed.
 *
 * The new selection is forwarded to the tileset model.
 */
void TilesetScene::set_selection_from_scene() {

  // Forward the change to the tileset.
  QItemSelection selection;
  for (QGraphicsItem* item : selectedItems()) {
    QModelIndex index = model.index(item->data(0).toInt());
    selection.select(index, index);
  }

  model.get_selection().select(selection, QItemSelectionModel::ClearAndSelect);
}
