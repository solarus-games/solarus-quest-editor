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
 * @param rect The exposed rectangle.
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

  if (model.get_patterns_image().isNull()) {
    // The tileset image does not exist yet.
    // Maybe this is a recently created tileset.
    QString path = get_quest().get_tileset_tiles_image_path(model.get_tileset_id());
    path = path.right(path.length() - get_quest().get_data_path().length() - 1);
    addText(tr("Missing tileset image '%1'").arg(path));
    return;
  }

  setSceneRect(QRectF(QPoint(0, 0), model.get_patterns_image().size()));
  const QMap<QString, QRect>& patterns_frame = model.get_patterns_frame();
  for (auto it = patterns_frame.constBegin(); it != patterns_frame.constEnd(); ++it) {
    const QString& pattern_id = it.key();
    const QRect& frame = it.value();
    QPixmap pattern_image = model.get_pattern_image(pattern_id);
    QGraphicsPixmapItem* pattern_item = addPixmap(pattern_image);
    pattern_item->setPos(frame.topLeft());
    pattern_item->setFlags(
          QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
  }
}
