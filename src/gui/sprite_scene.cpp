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
#include "gui/gui_tools.h"
#include "gui/sprite_scene.h"
#include "quest.h"
#include "sprite_model.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QPalette>
#include <QStyleOptionGraphicsItem>
#include <memory>

/**
 * @brief Graphic item representing a direction.
 */
class DirectionItem : public QGraphicsItem {

public:

  DirectionItem(SpriteModel& model, const SpriteModel::Index& index);

  // Enable the use of qgraphicsitem_cast with this item.
  enum {
    Type = UserType + 1
  };

  virtual int type() const override {
    return Type;
  }

  SpriteModel::Index get_index() const;
  void set_index(const SpriteModel::Index& index);

  virtual QRectF boundingRect() const override;

protected:

  virtual void paint(QPainter* painter,
                     const QStyleOptionGraphicsItem* option,
                     QWidget* widget = nullptr) override;

private:

  SpriteModel& model;            /**< The sprite this direction belongs to. */
  SpriteModel::Index index;      /**< Index of the direction in the sprite. */

};

/**
 * @brief Creates a sprite scene.
 * @param model The sprite data to represent in the scene.
 * @param parent The parent object or nullptr.
 */
SpriteScene::SpriteScene(SpriteModel& model, QObject* parent) :
  QGraphicsScene(parent),
  model(model) {

  rebuild();

  // Synchronize the scene selection with the sprite selection model.
  connect(&model.get_selection_model(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(update_selection_to_scene(QItemSelection, QItemSelection)));
  connect(this, SIGNAL(selectionChanged()),
          this, SLOT(set_selection_from_scene()));

  // Watch direction geometry changes.
  connect(&model, SIGNAL(direction_position_changed(Index,QPoint)),
          this, SLOT(update_direction_position(Index,QPoint)));
  connect(&model, SIGNAL(direction_size_changed(Index,QSize)),
          this, SLOT(invalidate()));
  connect(&model, SIGNAL(direction_num_frames_changed(Index,int)),
          this, SLOT(invalidate()));
  connect(&model, SIGNAL(direction_num_columns_changed(Index,int)),
          this, SLOT(invalidate()));
  connect(&model, SIGNAL(animation_image_changed(Index,QString)),
          this, SLOT(update_image()));
}

/**
 * @brief Returns the sprite represented in the scene.
 * @return The sprite.
 */
const SpriteModel& SpriteScene::get_model() const {
  return model;
}

/**
 * @brief Returns the quest the sprite belongs to.
 * @return The quest.
 */
const Quest& SpriteScene::get_quest() const {
  return model.get_quest();
}

/**
 * @brief Returns the index of the direction represented by an item.
 * @param item A graphic item.
 * @return The direction index, or invalid index if this is not a
 * direction item of the scene.
 */
SpriteModel::Index SpriteScene::get_direction_index(
    const QGraphicsItem* item) {

  const DirectionItem* direction_item =
      qgraphicsitem_cast<const DirectionItem*>(item);
  if (direction_item == nullptr) {
    return SpriteModel::Index();
  }

  return direction_item->get_index();
}

/**
 * @brief Draws the image of selected animation as background.
 * @param painter The painter.
 * @param rect The exposed rectangle in scene coordinates.
 * It may be larger than the scene.
 */
void SpriteScene::drawBackground(QPainter* painter, const QRectF& rect) {

  // Draw the background color.
  painter->fillRect(rect, palette().window());

  // Draw the full PNG image of the sprite animation.
  const QImage& src_image = model.get_animation_image(animation_name);
  if (!src_image.isNull()) {
    painter->setOpacity(0.5);
    painter->drawImage(0, 0, src_image);
  }
}

/**
 * @brief Create all directions items of the selected animation in the scene.
 */
void SpriteScene::rebuild() {

  // Get the selected index
  SpriteModel::Index index = model.get_selected_index();
  animation_name = index.animation_name;

  // Remove existing direction items
  QSignalBlocker blocker(this);
  for (auto& item: direction_items) {
    removeItem(item);
  }
  direction_items.clear();
  blocker.unblock();

  // Update image
  update_image();

  // Create direction items
  int num_directions = model.get_animation_num_directions(index);
  SpriteModel::Index direction_index = index;
  for (int nb = 0; nb < num_directions; ++nb) {

    direction_index.direction_nb = nb;
    DirectionItem* direction_item = new DirectionItem(model, direction_index);
    bool selected = nb == index.direction_nb;
    direction_item->setSelected(selected);
    direction_item->setZValue(selected);
    addItem(direction_item);
    direction_items.append(direction_item);
  }
}

/**
 * @brief Slot called when the source image of the animation has changed.
 */
void SpriteScene::update_image() {

  const QImage& image = model.get_animation_image(animation_name);

  if (image.isNull()) {
    QString src_image = model.get_animation_source_image(animation_name);
    if (!src_image.isEmpty()) {
      QString path = get_quest().get_sprite_image_path(src_image);
      path = path.right(path.length() - get_quest().get_data_path().length() - 1);
      addText(tr("Missing source image '%1'").arg(path));
    }
  }

  setSceneRect(QRectF(QPoint(0, 0), image.size()));
  invalidate();
}

/**
 * @brief Slot called when the sprite selection has changed.
 *
 * The selection on the scene is updated.
 */
void SpriteScene::update_selection_to_scene(
    const QItemSelection& /* selected */,
    const QItemSelection& /* deselected */) {

  SpriteModel::Index index = model.get_selected_index();
  if (index.animation_name != animation_name) {
    rebuild();
  } else if (index.is_valid()) {

    QSignalBlocker blocker(this);

    // Update the scene with the change.
    for (int nb = 0; nb < direction_items.size(); ++nb) {
      bool selected = index.direction_nb == nb;
      direction_items[nb]->setSelected(selected);
      direction_items[nb]->setZValue(selected);
    }

    blocker.unblock();
  }
}

/**
 * @brief Slot called when the scene selection has changed.
 *
 * The new selection is forwarded to the sprite model.
 */
void SpriteScene::set_selection_from_scene() {

  // Forward the change to the sprite.
  QList<QGraphicsItem*> items = selectedItems();

  if (items.size() == 0) {
    model.set_selected_animation(animation_name);
    return;
  }

  DirectionItem* direction_item = qgraphicsitem_cast<DirectionItem*>(items[0]);
  SpriteModel::Index index = direction_item->get_index();
  if (index.direction_nb != model.get_selected_index().direction_nb) {
    model.set_selected_index(index);
  }
}

/**
 * @brief Slot called when the position of a direction changes.
 * @param index Index of the direction changed.
 * @param pos The new position of the direction.
 */
void SpriteScene::update_direction_position(
    const Index &index, const QPoint &pos) {

  if (index.animation_name != animation_name ||
      index.direction_nb >= direction_items.size()) {
    return;
  }

  DirectionItem* direction_item =
      qgraphicsitem_cast<DirectionItem*>(direction_items[index.direction_nb]);
  if (direction_item != nullptr) {
    direction_item->setPos(pos);
  }
}

/**
 * @brief Creates a direction item.
 * @param model The sprite.
 * @param index Index of the direction in the sprite.
 */
DirectionItem::DirectionItem(
    SpriteModel& model, const SpriteModel::Index& index) :
  model(model),
  index(index) {

  QRect frame = model.get_direction_frame_rect(index);
  setPos(frame.topLeft());
  setFlags(ItemIsSelectable | ItemIsFocusable);
}

/**
 * @brief Returns the index of the direction in the sprite.
 * @return The direction index.
 */
SpriteModel::Index DirectionItem::get_index() const {
  return index;
}

/**
 * @brief Sets the index of the direction in the sprite.
 * @param index The new index.
 */
void DirectionItem::set_index(const SpriteModel::Index& index) {
  this->index = index;
}

/**
 * @brief Returns the bounding rect of this direction item.
 * @return The bounding rect.
 */
QRectF DirectionItem::boundingRect() const {

  QRect rect = model.get_direction_all_frames_rect(index);
  QPoint top_left = rect.topLeft();
  rect.translate(-top_left);
  return rect;
}

/**
 * @brief Paints the direction item.
 *
 * Reimplemented to draw our own selection marker.
 *
 * @param painter The painter.
 * @param option Style option of the item.
 * @param widget The widget being painted or nullptr.
 */
void DirectionItem::paint(QPainter* painter,
                        const QStyleOptionGraphicsItem* option,
                        QWidget* widget) {

  if (!model.direction_exists(index)) {
    // Bug in the editor.
    qCritical() << SpriteScene::tr("No such direction index: %1").arg(index.direction_nb);
    return;
  }

  QRect box = model.get_direction_frame_rect(index);
  QPoint top_left = box.topLeft();
  box.translate(-top_left);

  // Start with an opaque background, to erase anything below
  // if the direction has transparency.
  painter->fillRect(box, widget->palette().window());

  const bool selected = option->state & QStyle::State_Selected;

  // First, paint the item like if there was no selection, to avoid
  // Qt's built-in selection marker.
  QList<QRect> frames = model.get_direction_frames(index);
  QList<QPixmap> pixmaps = model.get_direction_all_frames(index);

  for (int i = 0; i < frames.size() && i < pixmaps.size(); i++) {

    QRect& frame = frames[i];
    frame.translate(-top_left);

    painter->drawPixmap(frame, pixmaps[i]);

    // Add our selection marker if is selected.
    if (selected) {
      GuiTools::draw_rectangle_outline(*painter, frame, Qt::blue, 1);
    }
  }
}
