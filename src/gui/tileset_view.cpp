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
#include "gui/tileset_view.h"
#include "gui/tileset_model.h"
#include <QGraphicsPixmapItem>

/**
 * @brief Creates a tileset view.
 * @param parent The parent widget or nullptr.
 */
TilesetView::TilesetView(QWidget* parent) :
  QGraphicsView(parent) {

  setBackgroundBrush(palette().window());
  setAlignment(Qt::AlignTop | Qt::AlignLeft);
  scale(2.0, 2.0);  // Initial zoom: x2.
}

/**
 * @brief Sets the tileset to represent in this view.
 * @param model The tileset model.
 */
void TilesetView::set_model(TilesetModel* model) {

  this->model = model;

  build();  // Create the scene from the model.
}

/**
 * @brief Draws the tileset image as background.
 * @param painter The painter.
 * @param rect The exposed rectangle.
 */
void TilesetView::drawBackground(QPainter* painter, const QRectF& /* rect */) {

  // Draw the background color.
  painter->fillRect(painter->viewport(), palette().window());

  // Draw the full PNG image of the tileset.
  painter->drawImage(0, 0, model->get_patterns_image());
}

/**
 * @brief Create all patterns items in the view.
 */
void TilesetView::build() {

  if (model == nullptr) {
    setScene(nullptr);
    return;
  }

  QGraphicsScene* scene = new QGraphicsScene(this);
  setScene(scene);

  scene->setSceneRect(QRectF(QPoint(0, 0), model->get_patterns_image().size()));
  const QMap<QString, QRect>& patterns_frame = model->get_patterns_frame();
  for (auto it = patterns_frame.constBegin(); it != patterns_frame.constEnd(); ++it) {
    const QString& pattern_id = it.key();
    const QRect& frame = it.value();
    QPixmap pattern_image = model->get_pattern_image(pattern_id);
    QGraphicsPixmapItem* pattern_item = scene->addPixmap(pattern_image);
    pattern_item->setPos(frame.topLeft());
  }
}
